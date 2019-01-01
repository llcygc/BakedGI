#include "ProbeManager.h"


ProbeManager::ProbeManager()
{
}


ProbeManager::~ProbeManager()
{
}

void ProbeManager::SetUpProbes(Vector3 min, Vector3 max, Vector3 division, int resolution)
{
	m_probeDimension = division;
	int x = division.GetX();
	int y = division.GetY();
	int z = division.GetZ();

	m_probeCount = x * y * z;

	Vector3 dist = max - min;

	for(int i = 0; i < x; i++)
		for(int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
			{
				float xPos = (float)i / (x - 1);
				float yPos = (float)j / (y - 1);
				float zPos = (float)k / (z - 1);
				Probe probe;
				probe.position = min + Vector3(xPos, yPos, zPos) * dist;
				m_probes.push_back(probe);
			}

	int probeMapSlice = m_probeCount * 6;
	//Create Cubemap Probe Rendertargets to render scene probes
	m_irradianceMapCube.CreateArray(L"Irradiance Cube Map", resolution, resolution, probeMapSlice, DXGI_FORMAT_R11G11B10_FLOAT);
	m_normalMapCube.CreateArray(L"Normal Cube Map", resolution, resolution, probeMapSlice, DXGI_FORMAT_R16G16_UNORM);
	m_distanceMapCube.CreateArray(L"Distance Cube Map", resolution, resolution, probeMapSlice, DXGI_FORMAT_R16_FLOAT);
	m_distanceMapCube.SetClearColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	m_depthBuffer.Create(L"Cube Depth", resolution, resolution, DXGI_FORMAT_D32_FLOAT);

	//Create Octan textures for final probe trace
	m_irradianceMapOctan.CreateArray(L"Irradiance Octan Map", resolution, resolution, m_probeCount, DXGI_FORMAT_R11G11B10_FLOAT);
	m_normalMapCube.CreateArray(L"Normal Octan Map", resolution, resolution, m_probeCount, DXGI_FORMAT_R16G16_UNORM);
	m_distanceMapCube.CreateArray(L"Distance Octan Map", resolution, resolution, m_probeCount, DXGI_FORMAT_R16_FLOAT);

	SamplerDesc DefaultSamplerDesc;
	DefaultSamplerDesc.MaxAnisotropy = 8;

	m_debugDisplayRootSig.Reset(4, 1);
	m_debugDisplayRootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_debugDisplayRootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[3].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_debugDisplayRootSig.Finalize(L"Probe Debug Display", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_probeRenderRootSig.Reset(5, 2);
	m_probeRenderRootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_probeRenderRootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_probeRenderRootSig[4].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_probeRenderRootSig.Finalize(L"Probe Renderer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_cubeToOctanProjRootSig.Reset(4, 1);
	m_cubeToOctanProjRootSig.InitStaticSampler(0, DefaultSamplerDesc);
	m_cubeToOctanProjRootSig[0].InitAsConstants(0, 4);
	m_cubeToOctanProjRootSig[1].InitAsConstantBuffer(1);
	m_cubeToOctanProjRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
	m_cubeToOctanProjRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3);
	m_cubeToOctanProjRootSig.Finalize(L"Cube to Octan");

	m_computeTraceRootSig.Reset(4, 1);
	m_computeTraceRootSig.InitStaticSampler(0, DefaultSamplerDesc);
	m_computeTraceRootSig[0].InitAsConstants(0, 4);
	m_computeTraceRootSig[1].InitAsConstantBuffer(1);
	m_computeTraceRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_computeTraceRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
	m_computeTraceRootSig.Finalize(L"Probe Trace");

	m_traceTemporalRootSig.Reset(4, 1);
	m_traceTemporalRootSig.InitStaticSampler(0, DefaultSamplerDesc);
	m_traceTemporalRootSig[0].InitAsConstants(0, 4);
	m_traceTemporalRootSig[1].InitAsConstantBuffer(1);
	m_traceTemporalRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_traceTemporalRootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2);
	m_cubeToOctanProjRootSig.Finalize(L"Trace Temporal");
	
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
	m_probeRenderPSO.SetBlendState(BlendDisable);
	m_probeRenderPSO.SetDepthStencilState(DepthStateReadWrite);
	m_probeRenderPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_probeRenderPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_probeRenderPSO.SetRenderTargetFormats(3, ProbeFormats, m_depthBuffer.GetFormat());
	m_probeRenderPSO.SetVertexShader(g_pProbeRenderVS, sizeof(g_pProbeRenderVS));
	m_probeRenderPSO.SetPixelShader(g_pProbeRenderPS, sizeof(g_pProbeRenderPS));
	m_probeRenderPSO.Finalize();

	m_debugDisplayPSO.SetRootSignature(m_debugDisplayRootSig);
	m_probeRenderPSO.SetBlendState(BlendDisable);
	m_probeRenderPSO.SetDepthStencilState(DepthStateReadWrite);
	m_probeRenderPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_probeRenderPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_probeRenderPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), m_depthBuffer.GetFormat());
	m_probeRenderPSO.SetVertexShader(g_pProbeDebugVS, sizeof(g_pProbeDebugVS));
	m_probeRenderPSO.SetPixelShader(g_pProbeDebugPS, sizeof(g_pProbeDebugPS));
	m_probeRenderPSO.Finalize();

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

void ProbeManager::RenderProbes(GraphicsContext& gfxContext, Scene& scene, D3D12_VIEWPORT viewport, D3D12_RECT scissor, LightManager& lightManger)
{
	gfxContext.SetRootSignature(m_probeRenderRootSig);
	gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfxContext.TransitionResource(m_irradianceMapCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.TransitionResource(m_normalMapCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.TransitionResource(m_distanceMapCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	gfxContext.ClearColor(m_irradianceMapCube);
	gfxContext.ClearColor(m_normalMapCube);
	gfxContext.ClearColor(m_distanceMapCube);

	gfxContext.TransitionResource(m_depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	gfxContext.ClearDepth(m_depthBuffer);

	gfxContext.SetPipelineState(m_probeRenderPSO);

	for (int i = 0; i < 6; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[3] = { irradianceCubeRTVs[i], normalCubeRTVs[i], distanceCubeRTVs[i] };
		gfxContext.SetRenderTargets(3, rtvs, m_depthBuffer.GetDSV());
		gfxContext.SetViewportAndScissor(viewport, scissor);
		//RenderObjects(gfxContext, model, cam, kOpaque);
		scene.RenderScene(gfxContext, kOpaque);
	}
}

void ProbeManager::SetUpGpuDatas()
{

}

void ProbeManager::ReprojCubetoOctan()
{

}

void ProbeManager::CreateCubemapResouceViews()
{
	ID3D12Resource* irradCubeMapRes = m_irradianceMapCube.GetResource();
	ID3D12Resource* normalCubeMapRes = m_normalMapCube.GetResource();
	ID3D12Resource* distCubeMapRes = m_distanceMapCube.GetResource();

	ID3D12Device* device = Graphics::g_Device;
	for (int i = 0; i < 6; i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC irradRTDesc = {};
		irradRTDesc.Format = m_irradianceMapCube.GetFormat();
		irradRTDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		irradRTDesc.Texture2DArray.MipSlice = 0;
		irradRTDesc.Texture2DArray.FirstArraySlice = i;
		irradRTDesc.Texture2DArray.ArraySize = 1;
		irradianceCubeRTVs[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(irradCubeMapRes, &irradRTDesc, irradianceCubeRTVs[i]);

		D3D12_RENDER_TARGET_VIEW_DESC normalRTDesc = {};
		normalRTDesc.Format = m_normalMapCube.GetFormat();
		normalRTDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		normalRTDesc.Texture2DArray.MipSlice = 0;
		normalRTDesc.Texture2DArray.FirstArraySlice = i;
		normalRTDesc.Texture2DArray.ArraySize = 1;
		normalCubeRTVs[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(normalCubeMapRes, &normalRTDesc, normalCubeRTVs[i]);

		D3D12_RENDER_TARGET_VIEW_DESC distRTDesc = {};
		distRTDesc.Format = m_normalMapCube.GetFormat();
		distRTDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		distRTDesc.Texture2DArray.MipSlice = 0;
		distRTDesc.Texture2DArray.FirstArraySlice = i;
		distRTDesc.Texture2DArray.ArraySize = 1;
		distanceCubeRTVs[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(distCubeMapRes, &distRTDesc, distanceCubeRTVs[i]);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC irradianceSRVDesc = {};
	irradianceSRVDesc.Format = m_irradianceMapCube.GetFormat();
	irradianceSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	irradianceSRVDesc.Texture2DArray.MipLevels = 1;
	irradianceSRVDesc.Texture2DArray.MostDetailedMip = 0;
	irradianceSRVDesc.Texture2DArray.FirstArraySlice = 0;
	irradianceSRVDesc.Texture2DArray.ArraySize = (UINT)m_probeCount;
	irradianceCubeSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(irradCubeMapRes, &irradianceSRVDesc, irradianceCubeSRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC normalSRVDesc = {};
	normalSRVDesc.Format = m_normalMapCube.GetFormat();
	normalSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	normalSRVDesc.Texture2DArray.MipLevels = 1;
	normalSRVDesc.Texture2DArray.MostDetailedMip = 0;
	normalSRVDesc.Texture2DArray.FirstArraySlice = 0;
	normalSRVDesc.Texture2DArray.ArraySize = (UINT)m_probeCount;
	normalCubeSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(normalCubeMapRes, &irradianceSRVDesc, normalCubeSRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC distanceSRVDesc = {};
	distanceSRVDesc.Format = m_distanceMapCube.GetFormat();
	distanceSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	distanceSRVDesc.Texture2DArray.MipLevels = 1;
	distanceSRVDesc.Texture2DArray.MostDetailedMip = 0;
	distanceSRVDesc.Texture2DArray.FirstArraySlice = 0;
	distanceSRVDesc.Texture2DArray.ArraySize = (UINT)m_probeCount;
	distanceCubeSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(distCubeMapRes, &distanceSRVDesc, distanceCubeSRV);
}