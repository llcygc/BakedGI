//Core and Model Include
#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "CameraController.h"
#include "Model.h"
#include "Camera.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "ShadowCamera.h"
#include "GameInput.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "BufferManager.h"

//Project Include
#include "../Source/Graphics/LightManager.h"
#include "Resources/ResourceManager.h"
#include "../Source/Graphics/GIModules/ProbeManager.h"
#include "Graphics\DeferredRenderer.h"
#include "Scene\Scene.h"

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

	enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };

	D3D12_VIEWPORT m_MainViewport;
	D3D12_RECT m_MainScissor;

	Scene m_Scene;

	RootSignature m_RootSig;
	GraphicsPSO m_DepthPSO;
	GraphicsPSO m_CutoutDepthPSO;

	GraphicsPSO m_ModelPSO;
	GraphicsPSO m_CutoutModelPSO;

	GraphicsPSO m_GBufferPSO;
	GraphicsPSO m_GBufferCutoutPSO;

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

	LightManager m_LightManager;
	ProbeManager m_ProbeManager;
	DeferredRenderer m_DeferredRender;
	Vector3 m_SunDirection;
	ShadowCamera m_SunShadow;

	D3D12_CPU_DESCRIPTOR_HANDLE m_ExtraTextures;

	void RenderObjects(GraphicsContext& gfxContext, Matrix4 viewProjMatrix, eObjectFilter filter);

	void SetupLights();
	void SetupProbes(Model::BoundingBox boundingBox);
};

CREATE_APPLICATION( BakedGI )

ExpVar m_SunLightIntensity("Application/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
ExpVar m_AmbientIntensity("Application/Lighting/Ambient Intensity", 0.1f, -16.0f, 16.0f, 0.1f);
NumVar m_SunOrientation("Application/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f);
NumVar m_SunInclination("Application/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f);
NumVar ShadowDimX("Application/Lighting/Shadow Dim X", 5000, 1000, 10000, 100);
NumVar ShadowDimY("Application/Lighting/Shadow Dim Y", 3000, 1000, 10000, 100);
NumVar ShadowDimZ("Application/Lighting/Shadow Dim Z", 3000, 1000, 10000, 100);

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

	/*m_DepthPSO.SetRootSignature(m_RootSig);
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
	m_CutoutShadowPSO.SetRasterizerState(RasterizerShadowTwoSided);
	m_CutoutShadowPSO.Finalize();

	m_ModelPSO = m_DepthPSO;
	m_ModelPSO.SetBlendState(BlendDisable);
	m_ModelPSO.SetDepthStencilState(DepthStateReadWrite);
	m_ModelPSO.SetRenderTargetFormats(1, &ColorFormat, DepthFormat);
	m_ModelPSO.SetVertexShader(g_pClusterLightingShaderVS, sizeof(g_pClusterLightingShaderVS));
	m_ModelPSO.SetPixelShader(g_pClusterLightingShaderPS, sizeof(g_pClusterLightingShaderPS));
	m_ModelPSO.Finalize();

	m_CutoutModelPSO = m_ModelPSO;
	m_CutoutModelPSO.SetRasterizerState(RasterizerTwoSided);
	m_CutoutModelPSO.Finalize();*/

	m_ExtraTextures = g_ShadowBuffer.GetSRV();

	TextureManager::Initialize(ResourceManager::GetResourceRootPathWide() + L"Textures/");
	std::string modelPath = "Models/sponza.h3d";
	m_Scene.Initialize(modelPath);

	SetupLights();
	SetupProbes(m_Scene.GetSceneBoundingBox());
	m_DeferredRender.Initialize();
}

void BakedGI::SetupLights()
{
	m_LightManager.Initialize();
	LightData dirLight;

	float costheta = cosf(m_SunOrientation);
	float sintheta = sinf(m_SunOrientation);
	float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
	float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
	m_SunDirection = Normalize(Vector3(costheta * cosphi, sinphi, sintheta * cosphi));
	dirLight.colorAngle = Vector4(1.0f * m_SunLightIntensity, 1.0f * m_SunLightIntensity, 1.0f * m_SunLightIntensity, -1.0f);
	dirLight.forwardRange = Vector4(m_SunDirection.GetX(), m_SunDirection.GetY(), m_SunDirection.GetZ(), -1.0f);
	dirLight.positionType = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	dirLight.shadowParams = Vector4(0.0f, 0.0f, 1.0f / g_ShadowBuffer.GetWidth(), 0.0f);

	m_SunShadow.UpdateMatrix(-m_SunDirection, Vector3(0, -500.0f, 0), Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
		(uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);
	dirLight.worldToShadowMatrix = m_SunShadow.GetShadowMatrix();
	dirLight.viewProjMatrix = m_SunShadow.GetViewProjMatrix();
	m_LightManager.SetDirectionalLight(dirLight);
}

void BakedGI::SetupProbes(Model::BoundingBox box)
{
	m_ProbeManager.SetUpProbes(box.min, box.max, Vector3(4, 4, 4), 1024);
}

void BakedGI::Cleanup( void )
{
    // Free up resources in an orderly fashion
	m_DeferredRender.Release();
	m_LightManager.Release();
}

namespace Graphics
{
	extern EnumVar DebugZoom;
}

void BakedGI::Update( float deltaT )
{
    ScopedTimer _prof(L"Update State");

	if (GameInput::IsFirstPressed(GameInput::kLShoulder))
		DebugZoom.Decrement();
	else if (GameInput::IsFirstPressed(GameInput::kRShoulder))
		DebugZoom.Increment();

	m_MainViewport.TopLeftX = 0.5f;
	m_MainViewport.TopLeftY = 0.5f;
	m_MainViewport.Width = (float)g_SceneColorBuffer.GetWidth();
	m_MainViewport.Height = (float)g_SceneColorBuffer.GetHeight();
	m_MainViewport.MinDepth = 0.0f;
	m_MainViewport.MaxDepth = 1.0f;

	m_MainScissor.left = 0;
	m_MainScissor.top = 0;
	m_MainScissor.right = (LONG)g_SceneColorBuffer.GetWidth();
	m_MainScissor.bottom = (LONG)g_SceneColorBuffer.GetHeight();

	m_Scene.Update(deltaT);
	m_DeferredRender.Update();
}

void BakedGI::RenderScene( void )
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

	{
		m_LightManager.RenderShadows(gfxContext, m_Scene);
		m_DeferredRender.Render(gfxContext, m_Scene, m_MainViewport, m_MainScissor, m_LightManager);
	}

    // Rendering something

    gfxContext.Finish();
}
