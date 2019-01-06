#include "ProbeManager.h"


ProbeManager::ProbeManager()
{
}


ProbeManager::~ProbeManager()
{
}

void ProbeManager::SetUpProbes(Vector3 min, Vector3 max, float scale, Vector3 division, int resolution)
{
	m_debugSphere.Create();
	m_probeResolution = resolution;
	m_probeDimension = division;
	int x = division.GetX();
	int y = division.GetY();
	int z = division.GetZ();

	m_probeCount = x * y * z;

	Vector3 dist = max - min;

	Vector3 probeMin = min + dist * (1.0f - scale) / 2.0f;
	Vector3 probeMax = max - dist * (1.0f - scale) / 2.0f;
	Vector3 actualDist = probeMax - probeMin;

	XMStoreFloat4(&probeParams.probePosMin, Vector4(probeMin, 0.0f));
	XMStoreFloat4(&probeParams.probePosMax, Vector4(probeMax, 0.0f));
	XMStoreFloat4(&probeParams.probeZParam, Vector4(nearPlane, farPlane, 0.0f, 0.0f));

	float minMipRes = (float)m_probeResolution / MINMIP_SIZE;
	XMStoreFloat4(&probeParams.probeParam, Vector4(m_probeResolution, 1.0f / (float)m_probeResolution, minMipRes, 1.0f / minMipRes));
	XMStoreFloat4(&probeParams.screenParam, Vector4(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 0.0f, 0.0f));

	cubeCamRotations = new Quaternion[6]{ Quaternion(90, 0, 0), Quaternion(0, 90, 0), Quaternion(0, 0, 0), Quaternion(-90, 0, 0), Quaternion(0, -90, 0), Quaternion(0, 180, 0) };

	m_probes = new Probe[m_probeCount];

	for(int i = 0; i < x; i++)
		for(int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
			{
				float xPos = (float)i / (x - 1);
				float yPos = (float)j / (y - 1);
				float zPos = (float)k / (z - 1);
				Probe probe;
				probe.position = probeMin + Vector3(xPos, yPos, zPos) * actualDist;
				m_probes[k + j * z + i * y * z] = probe;
			}

	
	/*Probe probe;
	probe.position = (min + max) / 2.0f;
	m_probes[0] = probe;*/

	m_ProbeDataBuffer.Create(L"Probe Data Buffer", m_probeCount, sizeof(Probe), m_probes);

	int probeMapSlice = m_probeCount * 6;
	//Create Cubemap Probe Rendertargets to render scene probes
	m_irradianceMapCube.CreateArray(L"Irradiance Cube Map", resolution, resolution, probeMapSlice, DXGI_FORMAT_R11G11B10_FLOAT);
	m_normalMapCube.CreateArray(L"Normal Cube Map", resolution, resolution, probeMapSlice, DXGI_FORMAT_R16G16_UNORM);
	m_distanceMapCube.CreateArray(L"Distance Cube Map", resolution, resolution, probeMapSlice, DXGI_FORMAT_R16_UNORM);
	m_distanceMapCube.SetClearColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	m_depthBuffer.Create(L"Cube Depth", resolution, resolution, DXGI_FORMAT_D32_FLOAT);

	//Create Octan textures for final probe trace
	m_irradianceMapOctan.CreateArray(L"Irradiance Octan Map", resolution, resolution, m_probeCount, DXGI_FORMAT_R11G11B10_FLOAT);
	m_normalMapOctan.CreateArray(L"Normal Octan Map", resolution, resolution, m_probeCount, DXGI_FORMAT_R16G16_UNORM);
	m_distanceMapOctan.CreateArray(L"Distance Octan Map", resolution, resolution, m_probeCount, DXGI_FORMAT_R16_UNORM);
	m_distanceMinMipMapOctan.CreateArray(L"Distance Min Mip Octan Map", resolution / MINMIP_SIZE, resolution / MINMIP_SIZE, m_probeCount, DXGI_FORMAT_R16_UNORM);

	m_probeTraceBuffer.Create(L"Probe Trace Buffer", g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 1, DXGI_FORMAT_R11G11B10_FLOAT);
	m_probeTraceBufferLastFrame.Create(L"Probe Trace Buffer Last Frame", g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 1, DXGI_FORMAT_R11G11B10_FLOAT);

	CreateCubemapResouceViews();

	SamplerDesc DefaultSamplerDesc;
	DefaultSamplerDesc.MaxAnisotropy = 8;

	m_debugDisplayRootSig.Reset(5, 1);
	m_debugDisplayRootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_debugDisplayRootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 3, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[4].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_debugDisplayRootSig.Finalize(L"Probe Debug Display", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_probeRenderRootSig.Reset(6, 2);
	m_probeRenderRootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[0].InitAsConstants(1, 4, D3D12_SHADER_VISIBILITY_VERTEX);
	m_probeRenderRootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_probeRenderRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[3].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[4].InitAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[5].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig.Finalize(L"Probe Renderer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_cubeToOctanProjRootSig.Reset(4, 1);
	m_cubeToOctanProjRootSig.InitStaticSampler(0, DefaultSamplerDesc);
	m_cubeToOctanProjRootSig[0].InitAsConstants(0, 4);
	m_cubeToOctanProjRootSig[1].InitAsConstantBuffer(1);
	m_cubeToOctanProjRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
	m_cubeToOctanProjRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3);
	m_cubeToOctanProjRootSig.Finalize(L"Cube to Octan");

	m_computeTraceRootSig.Reset(5, 1);
	m_computeTraceRootSig.InitStaticSampler(0, DefaultSamplerDesc);
	m_computeTraceRootSig[0].InitAsConstants(0, 4);
	m_computeTraceRootSig[1].InitAsConstantBuffer(1);
	m_computeTraceRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_computeTraceRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5);
	m_computeTraceRootSig[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 4);
	m_computeTraceRootSig.Finalize(L"Probe Trace");

	m_traceTemporalRootSig.Reset(4, 1);
	m_traceTemporalRootSig.InitStaticSampler(0, DefaultSamplerDesc);
	m_traceTemporalRootSig[0].InitAsConstants(0, 4);
	m_traceTemporalRootSig[1].InitAsConstantBuffer(1);
	m_traceTemporalRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_traceTemporalRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
	m_traceTemporalRootSig.Finalize(L"Trace Temporal");
	
	D3D12_INPUT_ELEMENT_DESC vertElem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	
	DXGI_FORMAT ProbeFormats[3] = { m_irradianceMapCube.GetFormat(), m_normalMapCube.GetFormat(), m_distanceMapCube.GetFormat() };
	m_probeRenderPSO.SetRootSignature(m_probeRenderRootSig);
	m_probeRenderPSO.SetRasterizerState(RasterizerDefault);
	m_probeRenderPSO.SetBlendState(BlendDisable);
	m_probeRenderPSO.SetDepthStencilState(DepthStateReadWrite);
	m_probeRenderPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_probeRenderPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_probeRenderPSO.SetRenderTargetFormats(3, ProbeFormats, m_depthBuffer.GetFormat());
	m_probeRenderPSO.SetVertexShader(g_pProbeRenderVS, sizeof(g_pProbeRenderVS));
	m_probeRenderPSO.SetPixelShader(g_pProbeRenderPS, sizeof(g_pProbeRenderPS));
	m_probeRenderPSO.Finalize();

	m_debugDisplayPSO.SetRootSignature(m_debugDisplayRootSig);
	m_debugDisplayPSO.SetRasterizerState(RasterizerDefault);
	m_debugDisplayPSO.SetBlendState(BlendDisable);
	m_debugDisplayPSO.SetDepthStencilState(DepthStateReadWrite);
	m_debugDisplayPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_debugDisplayPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_debugDisplayPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), m_depthBuffer.GetFormat());
	m_debugDisplayPSO.SetVertexShader(g_pProbeDebugVS, sizeof(g_pProbeDebugVS));
	m_debugDisplayPSO.SetPixelShader(g_pProbeDebugPS, sizeof(g_pProbeDebugPS));
	m_debugDisplayPSO.Finalize();

	m_cubeToOctanPSO.SetRootSignature(m_cubeToOctanProjRootSig);
	m_cubeToOctanPSO.SetComputeShader(g_pCubeToOctanCS, sizeof(g_pCubeToOctanCS));
	m_cubeToOctanPSO.Finalize();

	m_computeMinMipPSO.SetRootSignature(m_cubeToOctanProjRootSig);
	m_computeMinMipPSO.SetComputeShader(g_pMinMipCS, sizeof(g_pMinMipCS));
	m_computeMinMipPSO.Finalize();

	m_computeTracePSO.SetRootSignature(m_computeTraceRootSig);
	m_computeTracePSO.SetComputeShader(g_pProbeTraceCS, sizeof(g_pProbeTraceCS));
	m_computeTracePSO.Finalize();

	m_tracTemporalPSO.SetRootSignature(m_traceTemporalRootSig);
	m_tracTemporalPSO.SetComputeShader(g_pTraceTemporalCS, sizeof(g_pTraceTemporalCS));
	m_tracTemporalPSO.Finalize();
}

void ProbeManager::RenderProbes(GraphicsContext& gfxContext, Scene& scene, LightManager& lightManger)
{
	XMStoreFloat4(&probeParams.probeParam, Vector4(m_probeDimension, m_probeCount));

	D3D12_VIEWPORT probeViewPort;
	probeViewPort.TopLeftX = 0.5f;
	probeViewPort.TopLeftY = 0.5f;
	probeViewPort.Width = (float)m_probeResolution;
	probeViewPort.Height = (float)m_probeResolution;
	probeViewPort.MinDepth = 0.0f;
	probeViewPort.MaxDepth = 1.0f;

	D3D12_RECT probeScissor;
	probeScissor.left = 0;
	probeScissor.top = 0;
	probeScissor.right = (LONG)m_probeResolution;
	probeScissor.bottom = (LONG)m_probeResolution;

	gfxContext.SetRootSignature(m_probeRenderRootSig);
	gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfxContext.TransitionResource(m_irradianceMapCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.TransitionResource(m_normalMapCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.TransitionResource(m_distanceMapCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.ClearColor(m_irradianceMapCube);
	gfxContext.ClearColor(m_normalMapCube);
	gfxContext.ClearColor(m_distanceMapCube);

	gfxContext.TransitionResource(m_depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

	gfxContext.SetPipelineState(m_probeRenderPSO);
	gfxContext.SetDynamicConstantBufferView(4, sizeof(probeParams), &probeParams);

	m_probeCamera.SetPerspectiveMatrix(3.14159f / 2.0f, 1.0f, nearPlane, farPlane);

	for (uint32_t probeID = 0; probeID < m_probeCount; probeID++)
	{
		m_probeCamera.SetPosition(m_probes[probeID].position);

		float x = m_probes[probeID].position.GetX();
		float y = m_probes[probeID].position.GetY();
		float z = m_probes[probeID].position.GetZ();

		Vector3 targets[6] = {
			Vector3(1, 0, 0),
			Vector3(-1, 0, 0),
			Vector3(0, -1, 0),
			Vector3(0, 1, 0),
			Vector3(0, 0, 1),
			Vector3(0, 0, -1)
		};

		Vector3 ups[6] = {
			Vector3(0, 1, 0),
			Vector3(0, 1, 0),
			Vector3(0, 0, 1),
			Vector3(0, 0, -1),
			Vector3(0, 1, 0),
			Vector3(0, 1, 0)
		};

		for (int faceID = 0; faceID < 6; faceID++)
		{
			//m_probeCamera.SetRotation(cubeCamRotations[faceID]);
			m_probeCamera.SetLookDirection(targets[faceID], -ups[faceID]);
			m_probeCamera.Update();
			//D3D12_CPU_DESCRIPTOR_HANDLE rtvs[3] = { m_irradianceMapCube.GetRTV(), m_normalMapOctan.GetRTV(), m_distanceMapOctan.GetRTV() };
			D3D12_CPU_DESCRIPTOR_HANDLE rtvs[3] = { CalSubRTV(m_irradianceMapCube, probeID, faceID), CalSubRTV(m_normalMapCube, probeID, faceID), CalSubRTV(m_distanceMapCube, probeID, faceID) };
			gfxContext.ClearDepth(m_depthBuffer);
			gfxContext.SetRenderTargets(3, rtvs, m_depthBuffer.GetDSV());
			gfxContext.SetViewportAndScissor(probeViewPort, probeScissor);
			gfxContext.SetConstants(0, (uint32_t)faceID, 0, 0, 0);
			//RenderObjects(gfxContext, model, cam, kOpaque);
			scene.RenderScene(gfxContext, m_probeCamera, kOpaque);
		}
	}

	{
		ScopedTimer _prof(L"Cube to Octan Projection", gfxContext);
		ComputeContext& cc = gfxContext.GetComputeContext();
		cc.TransitionResource(m_irradianceMapCube, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(m_normalMapCube, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(m_distanceMapCube, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		cc.TransitionResource(m_irradianceMapOctan, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cc.TransitionResource(m_normalMapOctan, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cc.TransitionResource(m_distanceMapOctan, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cc.TransitionResource(m_distanceMinMipMapOctan, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		cc.SetRootSignature(m_cubeToOctanProjRootSig);
		cc.SetPipelineState(m_cubeToOctanPSO);

		cc.SetDynamicConstantBufferView(1, sizeof(probeParams), &probeParams);
		
		D3D12_CPU_DESCRIPTOR_HANDLE uavs[4] = { m_irradianceMapOctan.GetUAV(), m_normalMapOctan.GetUAV(), m_distanceMapOctan.GetUAV(), m_distanceMinMipMapOctan.GetUAV() };
		cc.SetDynamicDescriptors(2, 0, 4, uavs);

		D3D12_CPU_DESCRIPTOR_HANDLE srvs[3] = { irradianceCubeSRV, normalCubeSRV, distanceCubeSRV };
		cc.SetDynamicDescriptors(3, 0, 3, srvs);
		cc.Dispatch3D(m_probeResolution, m_probeResolution, m_probeCount, 8, 8, 1);

		cc.SetRootSignature(m_cubeToOctanProjRootSig);
		cc.SetPipelineState(m_computeMinMipPSO);
		//Set rootsignatrue resources
		//D3D12_CPU_DESCRIPTOR_HANDLE uavs[4] = { m_irradianceMapOctan.GetUAV(), m_normalMapOctan.GetUAV(), m_distanceMapOctan.GetUAV(), m_distanceMinMipMapOctan.GetUAV() };
		cc.SetDynamicDescriptors(2, 0, 4, uavs);

		//D3D12_CPU_DESCRIPTOR_HANDLE srvs[3] = { irradianceCubeSRV, normalCubeSRV, distanceCubeSRV };
		cc.SetDynamicDescriptors(3, 0, 3, srvs);

		cc.Dispatch3D(m_probeResolution, m_probeResolution, m_probeCount, MINMIP_SIZE, MINMIP_SIZE, 1);
	}
}

void ProbeManager::ComputeTrace(ComputeContext& cc, D3D12_CPU_DESCRIPTOR_HANDLE* gBufferSRVs)
{
	{
		cc.TransitionResource(m_irradianceMapOctan, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(m_normalMapOctan, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(m_distanceMapOctan, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(m_distanceMinMipMapOctan, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		cc.SetRootSignature(m_computeTraceRootSig);
		cc.SetPipelineState(m_computeTracePSO);

		cc.SetDynamicConstantBufferView(1, sizeof(probeParams), &probeParams);
		cc.SetDynamicDescriptors(2, 0, 1, &m_probeTraceBuffer.GetUAV());
		//Set rootsignature resources
		D3D12_CPU_DESCRIPTOR_HANDLE srvs[5] = { m_irradianceMapOctan.GetSRV(), m_normalMapOctan.GetSRV(), m_distanceMapOctan.GetSRV(), m_distanceMinMipMapOctan.GetSRV(), m_ProbeDataBuffer.GetSRV() };
		D3D12_CPU_DESCRIPTOR_HANDLE gSrvs[4] = { gBufferSRVs[0], gBufferSRVs[1], gBufferSRVs[2], gBufferSRVs[3] };
		cc.SetDynamicDescriptors(3, 0, 5, srvs);
		cc.SetDynamicDescriptors(4, 0, 4, gSrvs);

		cc.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetWidth());
	}
}

void ProbeManager::SetUpGpuDatas()
{

}

void ProbeManager::ReprojCubetoOctan()
{

}

void ProbeManager::RenderDebugProbes(GraphicsContext& gfxContext, Scene& scene, D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	gfxContext.SetRootSignature(m_debugDisplayRootSig);
	gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ScopedTimer _prof(L"Render G-Buffers", gfxContext);
	gfxContext.TransitionResource(m_irradianceMapOctan, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfxContext.TransitionResource(m_normalMapOctan, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfxContext.TransitionResource(m_distanceMapOctan, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfxContext.TransitionResource(m_distanceMinMipMapOctan, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	gfxContext.SetPipelineState(m_debugDisplayPSO);
	gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
	D3D12_CPU_DESCRIPTOR_HANDLE srvs[4] = {m_irradianceMapOctan.GetSRV(), m_normalMapOctan.GetSRV(), m_distanceMapOctan.GetSRV(), m_distanceMinMipMapOctan.GetSRV()};
	gfxContext.SetDynamicDescriptors(2, 0, 4, srvs);

	D3D12_CPU_DESCRIPTOR_HANDLE cubeSRVs[3] = { irradianceCubeSRV, normalCubeSRV, distanceCubeSRV };
	gfxContext.SetDynamicDescriptors(3, 0, 3, cubeSRVs);

	Vector4 res(m_probeResolution, m_probeResolution, m_probeResolution / MINMIP_SIZE, m_probeResolution / MINMIP_SIZE);

	for (uint32_t probeID = 0; probeID < m_probeCount; probeID++)
	{
		XMStoreFloat4(&probeParams.probeParam, Vector4(m_probeDimension, probeID));
		gfxContext.SetDynamicConstantBufferView(1, sizeof(probeParams), &probeParams);

		Vector3 position = m_probes[probeID].position;
		Matrix3 scaleMat = Matrix3::MakeScale(100.0f);
		Matrix4 worldMatrix = Matrix4(AffineTransform(scaleMat, position));
		m_debugSphere.RenderPrimitive(gfxContext, scene.m_Camera, worldMatrix);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE ProbeManager::CalSubRTV(ColorBuffer& destBuffer, int probeID, int faceID)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle;
	destHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

	ID3D12Device* device = Graphics::g_Device;
	destHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	ID3D12Resource* destRes = destBuffer.GetResource();
	D3D12_RENDER_TARGET_VIEW_DESC destRTDesc = {};
	destRTDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	destRTDesc.Format = destBuffer.GetFormat();
	destRTDesc.Texture2DArray.MipSlice = 0;
	destRTDesc.Texture2DArray.FirstArraySlice = probeID * 6 + faceID;
	destRTDesc.Texture2DArray.ArraySize = 1;
	device->CreateRenderTargetView(destRes, &destRTDesc, destHandle);
	return destHandle;
}

void ProbeManager::CreateCubemapResouceViews()
{
	ID3D12Resource* irradCubeMapRes = m_irradianceMapCube.GetResource();
	ID3D12Resource* normalCubeMapRes = m_normalMapCube.GetResource();
	ID3D12Resource* distCubeMapRes = m_distanceMapCube.GetResource();

	ID3D12Device* device = Graphics::g_Device;
	
	D3D12_SHADER_RESOURCE_VIEW_DESC irradianceSRVDesc = {};
	irradianceSRVDesc.Format = m_irradianceMapCube.GetFormat();
	irradianceSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	irradianceSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	irradianceSRVDesc.TextureCubeArray.MipLevels = 1;
	irradianceSRVDesc.TextureCubeArray.MostDetailedMip = 0;
	irradianceSRVDesc.TextureCubeArray.First2DArrayFace = 0;
	irradianceSRVDesc.TextureCubeArray.NumCubes = (UINT)m_probeCount;
	irradianceCubeSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(irradCubeMapRes, &irradianceSRVDesc, irradianceCubeSRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC normalSRVDesc = {};
	normalSRVDesc.Format = m_normalMapCube.GetFormat();
	normalSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	normalSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	normalSRVDesc.TextureCubeArray.MipLevels = 1;
	normalSRVDesc.TextureCubeArray.MostDetailedMip = 0;
	normalSRVDesc.TextureCubeArray.First2DArrayFace = 0;
	normalSRVDesc.TextureCubeArray.NumCubes = (UINT)m_probeCount;
	normalCubeSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(normalCubeMapRes, &normalSRVDesc, normalCubeSRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC distanceSRVDesc = {};
	distanceSRVDesc.Format = m_distanceMapCube.GetFormat();
	distanceSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	distanceSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	distanceSRVDesc.TextureCubeArray.MipLevels = 1;
	distanceSRVDesc.TextureCubeArray.MostDetailedMip = 0;
	distanceSRVDesc.TextureCubeArray.First2DArrayFace = 0;
	distanceSRVDesc.TextureCubeArray.NumCubes = (UINT)m_probeCount;
	distanceCubeSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(distCubeMapRes, &distanceSRVDesc, distanceCubeSRV);
}

void ProbeManager::Release()
{
	m_irradianceMapCube.Destroy();
	m_normalMapCube.Destroy();
	m_distanceMapCube.Destroy();

	m_irradianceMapOctan.Destroy();
	m_normalMapOctan.Destroy();
	m_distanceMapOctan.Destroy();

	m_depthBuffer.Destroy();
	
	delete[] cubeCamRotations;
	delete[] m_probes;
}