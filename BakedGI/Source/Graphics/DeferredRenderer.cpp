#include "DeferredRenderer.h"


DeferredRenderer::DeferredRenderer()
{
}


DeferredRenderer::~DeferredRenderer()
{
}

void DeferredRenderer::Initialize(const Model& model)
{
	uint32_t width = g_SceneColorBuffer.GetWidth();
	uint32_t height = g_SceneColorBuffer.GetHeight();
	GBuffer0.Create(L"G-Buffer 0", width, height, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
	GBuffer1.Create(L"G-Buffer 1", width, height, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
	GBuffer2.Create(L"G-Buffer 2", width, height, 1, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

	SamplerDesc DefaultSamplerDesc;
	DefaultSamplerDesc.MaxAnisotropy = 8;

	m_GBufferSig.Reset(3, 1);
	m_GBufferSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_GBufferSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_GBufferSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_GBufferSig[2].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_GBufferSig.Finalize(L"G-Buffer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_LitSig.Reset(4, 0);
	m_LitSig[0].InitAsConstants(0, 4);
	m_LitSig[1].InitAsConstantBuffer(1);
	m_LitSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_LitSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 3);
	m_LitSig.Finalize(L"Deferred Lightinig");

	DXGI_FORMAT GBufferFormats[3] = { GBuffer0.GetFormat(), GBuffer1.GetFormat(), GBuffer2.GetFormat() };
	DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();

	D3D12_INPUT_ELEMENT_DESC vertElem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_GBufferPSO.SetRootSignature(m_GBufferSig);
	m_GBufferPSO.SetRasterizerState(RasterizerDefault);
	m_GBufferPSO.SetBlendState(BlendDisable);
	m_GBufferPSO.SetDepthStencilState(DepthStateReadWrite);
	m_GBufferPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_GBufferPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_GBufferPSO.SetRenderTargetFormats(3, GBufferFormats, DepthFormat);
	m_GBufferPSO.SetVertexShader(g_pGBufferShaderVS, sizeof(g_pGBufferShaderVS));
	m_GBufferPSO.SetPixelShader(g_pGBufferShaderPS, sizeof(g_pGBufferShaderPS));
	m_GBufferPSO.Finalize();

	m_DeferredLightingPSO.SetRootSignature(m_LitSig);
	m_DeferredLightingPSO.SetComputeShader(g_pDeferredLightingCS, sizeof(g_pDeferredLightingCS));
	m_DeferredLightingPSO.Finalize();

	m_pMaterialIsCutout.resize(model.m_Header.materialCount);
	for (uint32_t i = 0; i < model.m_Header.materialCount; ++i)
	{
		const Model::Material& mat = model.m_pMaterial[i];
		if (std::string(mat.texDiffusePath).find("thorn") != std::string::npos ||
			std::string(mat.texDiffusePath).find("plant") != std::string::npos ||
			std::string(mat.texDiffusePath).find("chain") != std::string::npos)
		{
			m_pMaterialIsCutout[i] = true;
		}
		else
		{
			m_pMaterialIsCutout[i] = false;
		}
	}
}

void DeferredRenderer::Update()
{

}

void DeferredRenderer::RenderObjects(GraphicsContext& gfxContext, const Model& model, const Camera& cam, eObjectFilter filter)
{
	struct VSConstants
	{
		//Matrix4 viewMatrix;
		//Matrix4 projMatrix;
		Matrix4 viewProjMatrix;
		//Matrix4 clusterMatrix;
		//Vector4 screenParam;
		//Vector4 projectionParam;
		XMFLOAT3 cameraPos;
	} perCameraConstants;

	perCameraConstants.viewProjMatrix = cam.GetViewProjMatrix();
	XMStoreFloat3(&perCameraConstants.cameraPos, cam.GetPosition());

	gfxContext.SetDynamicConstantBufferView(0, sizeof(perCameraConstants), &perCameraConstants);

	uint32_t materialIdx = 0xFFFFFFFFul;

	uint32_t VertexStride = model.m_VertexStride;

	for (uint32_t meshIndex = 0; meshIndex < model.m_Header.meshCount; meshIndex++)
	{
		const Model::Mesh& mesh = model.m_pMesh[meshIndex];

		uint32_t indexCount = mesh.indexCount;
		uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
		uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

		if (mesh.materialIndex != materialIdx)
		{
			if (m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kCutout) ||
				!m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kOpaque))
				continue;

			materialIdx = mesh.materialIndex;
			gfxContext.SetDynamicDescriptors(1, 0, 6, model.GetSRVs(materialIdx));
		}

		gfxContext.SetConstants(2, baseVertex, materialIdx);
		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
	}
}

void DeferredRenderer::Render(GraphicsContext& gfxContext, const Model& model, const Camera& cam, D3D12_VIEWPORT viewport, D3D12_RECT scissor)
{
	auto& pfnSetupGraphicsState = [&](void)
	{
		gfxContext.SetRootSignature(m_GBufferSig);
		gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfxContext.SetIndexBuffer(model.m_IndexBuffer.IndexBufferView());
		gfxContext.SetVertexBuffer(0, model.m_VertexBuffer.VertexBufferView());
	};

	pfnSetupGraphicsState();

	{
		ScopedTimer _prof(L"Render G-Buffers", gfxContext);
		gfxContext.TransitionResource(GBuffer0, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		gfxContext.TransitionResource(GBuffer1, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		gfxContext.TransitionResource(GBuffer2, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		gfxContext.ClearColor(GBuffer0);
		gfxContext.ClearColor(GBuffer1);
		gfxContext.ClearColor(GBuffer2);

		gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		gfxContext.ClearDepth(g_SceneDepthBuffer);

		gfxContext.SetPipelineState(m_GBufferPSO);
		D3D12_CPU_DESCRIPTOR_HANDLE gBufferRTVs[3] = { GBuffer0.GetRTV(), GBuffer1.GetRTV(), GBuffer2.GetRTV() };
		gfxContext.SetRenderTargets(3, gBufferRTVs, g_SceneDepthBuffer.GetDSV());

		gfxContext.SetViewportAndScissor(viewport, scissor);
		RenderObjects(gfxContext, model, cam, kOpaque);
	}

	{
		ScopedTimer _prof(L"Deferred Lighting Compute", gfxContext);
		ComputeContext& cc = gfxContext.GetComputeContext();

		cc.TransitionResource(GBuffer0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(GBuffer1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(GBuffer2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		cc.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		cc.SetRootSignature(m_LitSig);
		cc.SetPipelineState(m_DeferredLightingPSO);
		D3D12_CPU_DESCRIPTOR_HANDLE gBufferSRVs[3] = { GBuffer0.GetSRV(), GBuffer1.GetSRV(), GBuffer2.GetSRV() };
		cc.SetDynamicDescriptors(2, 0, 1, &g_SceneColorBuffer.GetUAV());
		cc.SetDynamicDescriptors(3, 0, 3, gBufferSRVs);
		cc.Dispatch2D(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());
	}

	gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
}

void DeferredRenderer::Release()
{
	GBuffer0.Destroy();
	GBuffer1.Destroy();
	GBuffer2.Destroy();
}
