#include "ShadowManager.h"



ShadowManager::ShadowManager()
{
}


ShadowManager::~ShadowManager()
{
}

void ShadowManager::Initialize()
{
	SamplerDesc DefaultSamplerDesc;
	DefaultSamplerDesc.MaxAnisotropy = 8;

	m_ShadowRootSig.Reset(5, 2);
	m_ShadowRootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ShadowRootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ShadowRootSig[0].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_ShadowRootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_ShadowRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ShadowRootSig[3].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ShadowRootSig[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ShadowRootSig.Finalize(L"Render Shadow", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	D3D12_INPUT_ELEMENT_DESC vertElem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_ShadowPSO.SetRootSignature(m_ShadowRootSig);
	m_ShadowPSO.SetRasterizerState(RasterizerDefault);
	m_ShadowPSO.SetBlendState(BlendNoColorWrite);
	m_ShadowPSO.SetDepthStencilState(DepthStateReadWrite);
	m_ShadowPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_ShadowPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
	m_ShadowPSO.SetVertexShader(g_pDepthShaderVS, sizeof(g_pDepthShaderVS));
	m_ShadowPSO.Finalize();

	m_CutoutShadowPSO = m_ShadowPSO;
	m_CutoutShadowPSO.SetPixelShader(g_pDepthShaderPS, sizeof(g_pDepthShaderPS));
	m_CutoutShadowPSO.SetRasterizerState(RasterizerShadowTwoSided);
	m_CutoutShadowPSO.Finalize();
}

void ShadowManager::Render(GraphicsContext& gfxContext, Scene& scene, Matrix4 shadowMatrix)
{
	gfxContext.SetRootSignature(m_ShadowRootSig);
	gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	{
		ScopedTimer _prof(L"Render Shadow Map", gfxContext);

		g_ShadowBuffer.BeginRendering(gfxContext);
		gfxContext.SetPipelineState(m_ShadowPSO);
		scene.RenderScene(gfxContext, shadowMatrix, kOpaque);
		g_ShadowBuffer.EndRendering(gfxContext);
	}
}
