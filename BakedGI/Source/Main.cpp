//Core and Model Include
#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "CameraController.h"
#include "Model.h"
#include "Camera.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "GameInput.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "BufferManager.h"

//Project Include
#include "Resources/ResourceManager.h"

#include "CompiledShaders/ClusterLightingShaderVS.h"
#include "CompiledShaders/ClusterLightingShaderPS.h"
#include "CompiledShaders/DepthShaderVS.h"
#include "CompiledShaders/DepthShaderPS.h"
#include "CompiledShaders/ShadowCasterShaderVS.h"
#include "CompiledShaders/ShadowCasterShaderPS.h"

using namespace GameCore;
using namespace Graphics;

class BakedGI : public GameCore::IGameApp
{
public:

    BakedGI()
    {
    }

    virtual void Startup( void ) override;
    virtual void Cleanup( void ) override;

    virtual void Update( float deltaT ) override;
    virtual void RenderScene( void ) override;

private:

	Camera m_Camera;
	std::auto_ptr<CameraController> m_CameraController;

	Model m_Model;

	RootSignature m_RootSig;
	GraphicsPSO m_DepthPSO;
	GraphicsPSO m_CutoutDepthPSO;

	GraphicsPSO m_ModelPSO;
	GraphicsPSO m_CutoutDephtPSO;

	GraphicsPSO m_ShadowPSO;
	GraphicsPSO m_CutoutShadowPSO;

	D3D12_CPU_DESCRIPTOR_HANDLE m_DefaultSampler;
	D3D12_CPU_DESCRIPTOR_HANDLE m_ShadowSampler;
	D3D12_CPU_DESCRIPTOR_HANDLE m_BiasedDefaultSampler;

	D3D12_SHADER_BYTECODE m_DepthVS;
	D3D12_SHADER_BYTECODE m_ShadowCastVS;
	D3D12_SHADER_BYTECODE m_ClusterLightingVS;

	D3D12_SHADER_BYTECODE m_DepthPS;
	D3D12_SHADER_BYTECODE m_ShadowCastPS;
	D3D12_SHADER_BYTECODE m_ClusterLightingPS;

	D3D12_SHADER_BYTECODE m_ClusterLightCS;
};

CREATE_APPLICATION( BakedGI )

void BakedGI::Startup( void )
{
	SamplerDesc DefaultSamplerDesc;
	DefaultSamplerDesc.MaxAnisotropy = 8;

	m_RootSig.Reset(5, 2);
	m_RootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig.InitStaticSampler(1, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 64, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig[4].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig.Finalize(L"ModelViewer", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
	DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
	DXGI_FORMAT ShadowFormat = g_ShadowBuffer.GetFormat();

	D3D12_INPUT_ELEMENT_DESC vertElem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	m_DepthPSO.SetRootSignature(m_RootSig);
	m_DepthPSO.SetRasterizerState(RasterizerDefault);
	m_DepthPSO.SetBlendState(BlendNoColorWrite);
	m_DepthPSO.SetDepthStencilState(DepthStateReadWrite);
	m_DepthPSO.SetInputLayout(_countof(vertElem), vertElem);
	m_DepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	m_DepthPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
	m_DepthPSO.SetVertexShader(g_pDepthShaderVS, sizeof(g_pDepthShaderVS));
	m_DepthPSO.Finalize();

	m_CutoutDepthPSO = m_DepthPSO;
	m_CutoutDepthPSO.SetPixelShader(g_pDepthShaderPS, sizeof(g_pDepthShaderPS));
	m_CutoutDepthPSO.SetRasterizerState(RasterizerTwoSided);
	m_CutoutDepthPSO.Finalize();

	m_ShadowPSO = m_DepthPSO;
	m_ShadowPSO.SetRasterizerState(RasterizerShadow);
	m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
	m_ShadowPSO.Finalize();

	m_CutoutShadowPSO = m_DepthPSO;
	m_CutoutShadowPSO.SetPixelShader(g_pDepthShaderPS, sizeof(g_pDepthShaderPS));
	m_CutoutShadowPSO.SetRasterizerState(RasterizerTwoSided);
	m_CutoutShadowPSO.Finalize();

	m_ModelPSO = m_DepthPSO;
	m_ModelPSO.SetBlendState(BlendDisable);
	
	void* clusterLightingShaderVS = ResourceManager::LoadShader("Shaders/ClusterLightingShaderVS.cso");
	m_ClusterLightingVS = CD3DX12_SHADER_BYTECODE(clusterLightingShaderVS, sizeof(&clusterLightingShaderVS));
	TextureManager::Initialize(ResourceManager::GetResourceRootPathWide() + L"Textures/");
	std::string modelPath = "Models/sponza.h3d";
	ASSERT(m_Model.Load((ResourceManager::GetResourceRootPath() + modelPath).c_str()));
	ASSERT(m_Model.m_Header.meshCount > 0, "Model has no mesh in it");

	float modelRadius = Length(m_Model.m_Header.boundingBox.max - m_Model.m_Header.boundingBox.min) * .5f;
	const Vector3 eye = (m_Model.m_Header.boundingBox.min + m_Model.m_Header.boundingBox.max) * .5f + Vector3(modelRadius * .5f, 0.0f, 0.0f);
	m_Camera.SetEyeAtUp(eye, Vector3(kZero), Vector3(kYUnitVector));
	m_Camera.SetZRange(1.0f, 10000.0f);
	m_CameraController.reset(new CameraController(m_Camera, Vector3(kYUnitVector)));
}

void BakedGI::Cleanup( void )
{
    // Free up resources in an orderly fashion
}

void BakedGI::Update( float deltaT )
{
    ScopedTimer _prof(L"Update State");

    // Update something
}

void BakedGI::RenderScene( void )
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.ClearColor(g_SceneColorBuffer);
    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    // Rendering something

    gfxContext.Finish();
}
