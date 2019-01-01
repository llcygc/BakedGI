#include "DeferredRenderer.h"


DeferredRenderer::DeferredRenderer()
{
}


DeferredRenderer::~DeferredRenderer()
{
}

void DeferredRenderer::Initialize()
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
	m_GBufferSig[0].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_GBufferSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_GBufferSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_GBufferSig.Finalize(L"G-Buffer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	m_LitSig.Reset(6, 1);
	m_LitSig.InitStaticSampler(0, SamplerShadowDesc);
	m_LitSig[0].InitAsConstants(0, 4);
	m_LitSig[1].InitAsConstantBuffer(1);
	m_LitSig[2].InitAsConstantBuffer(2);
	m_LitSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_LitSig[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
	m_LitSig[5].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 1);
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
}

void DeferredRenderer::Update()
{

}

void DeferredRenderer::Render(GraphicsContext& gfxContext, Scene& scene, D3D12_VIEWPORT viewport, D3D12_RECT scissor, LightManager& lightManger)
{
	/*auto& pfnSetupGraphicsState = [&](void)
	{
		gfxContext.SetIndexBuffer(model.m_IndexBuffer.IndexBufferView());
		gfxContext.SetVertexBuffer(0, model.m_VertexBuffer.VertexBufferView());
	};*/


	{
		gfxContext.SetRootSignature(m_GBufferSig);
		gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
		//RenderObjects(gfxContext, model, cam, kOpaque);
		scene.RenderScene(gfxContext, kOpaque);
	}

	{
		ScopedTimer _prof(L"Deferred Lighting Compute", gfxContext);
		ComputeContext& cc = gfxContext.GetComputeContext();

		cc.TransitionResource(GBuffer0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(GBuffer1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(GBuffer2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cc.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		cc.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		struct CSConstants
		{
			//Matrix4 viewMatrix;
			//Matrix4 projMatrix;
			Matrix4 invViewProjMatrix;
			//Matrix4 clusterMatrix;
			//Vector4 screenParam;
			//Vector4 projectionParam;
			XMFLOAT4 screenParams;
			XMFLOAT3 cameraPos;
		} deferredConstants;

		Vector4 screenParams(g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight(), 0, 0);
		deferredConstants.invViewProjMatrix = Invert(scene.m_Camera.GetViewProjMatrix());
		XMStoreFloat4(&deferredConstants.screenParams, screenParams);
		XMStoreFloat3(&deferredConstants.cameraPos, scene.m_Camera.GetPosition());

		cc.SetRootSignature(m_LitSig);
		cc.SetPipelineState(m_DeferredLightingPSO);

		cc.SetDynamicConstantBufferView(1, sizeof(deferredConstants), &deferredConstants);
		//Set lights data
		lightManger.PrepareLightsDataForGPUCS(cc, 2);
		//Set final HDR render target
		cc.SetDynamicDescriptors(3, 0, 1, &g_SceneColorBuffer.GetUAV());
		//Set GBuffer textures
		D3D12_CPU_DESCRIPTOR_HANDLE gBufferSRVs[4] = { GBuffer0.GetSRV(), GBuffer1.GetSRV(), GBuffer2.GetSRV(), g_SceneDepthBuffer.GetDepthSRV() };
		cc.SetDynamicDescriptors(4, 0, 4, gBufferSRVs);
		//Set shadow texture
		cc.SetDynamicDescriptors(5, 0, 1, &g_ShadowBuffer.GetSRV());

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
