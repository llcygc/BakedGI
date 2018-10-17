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

	enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };

	Camera m_Camera;
	std::auto_ptr<CameraController> m_CameraController;
	Matrix4 m_ViewProjMatrix;
	D3D12_VIEWPORT m_MainViewport;
	D3D12_RECT m_MainScissor;

	Model m_Model;
	std::vector<bool> m_pMaterialIsCutout;

	RootSignature m_RootSig;
	GraphicsPSO m_DepthPSO;
	GraphicsPSO m_CutoutDepthPSO;

	GraphicsPSO m_ModelPSO;
	GraphicsPSO m_CutoutModelPSO;

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

	void RenderObjects(GraphicsContext& gfxContext, Matrix4 viewProjMatrix, eObjectFilter filter);
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
	m_ModelPSO.SetDepthStencilState(DepthStateReadWrite);
	m_ModelPSO.SetRenderTargetFormats(1, &ColorFormat, DepthFormat);
	m_ModelPSO.SetVertexShader(g_pClusterLightingShaderVS, sizeof(g_pClusterLightingShaderVS));
	m_ModelPSO.SetPixelShader(g_pClusterLightingShaderPS, sizeof(g_pClusterLightingShaderPS));
	m_ModelPSO.Finalize();

	m_CutoutModelPSO = m_ModelPSO;
	m_CutoutModelPSO.SetRasterizerState(RasterizerTwoSided);
	m_CutoutModelPSO.Finalize();


	TextureManager::Initialize(ResourceManager::GetResourceRootPathWide() + L"Textures/");
	std::string modelPath = "Models/sponza.h3d";
	ASSERT(m_Model.Load((ResourceManager::GetResourceRootPath() + modelPath).c_str()));
	ASSERT(m_Model.m_Header.meshCount > 0, "Model has no mesh in it");

	m_pMaterialIsCutout.resize(m_Model.m_Header.materialCount);
	for (uint32_t i = 0; i < m_Model.m_Header.materialCount; ++i)
	{
		const Model::Material& mat = m_Model.m_pMaterial[i];
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

	float modelRadius = Length(m_Model.m_Header.boundingBox.max - m_Model.m_Header.boundingBox.min) * .5f;
	const Vector3 eye = (m_Model.m_Header.boundingBox.min + m_Model.m_Header.boundingBox.max) * .5f + Vector3(modelRadius * .5f, 0.0f, 0.0f);
	m_Camera.SetEyeAtUp(eye, Vector3(kZero), Vector3(kYUnitVector));
	m_Camera.SetZRange(1.0f, 10000.0f);
	m_CameraController.reset(new CameraController(m_Camera, Vector3(kYUnitVector)));
}

void BakedGI::Cleanup( void )
{
    // Free up resources in an orderly fashion
	m_Model.Clear();
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

	m_CameraController->Update(deltaT);
	m_ViewProjMatrix = m_Camera.GetViewProjMatrix();

	m_MainViewport.Width = (float)g_SceneColorBuffer.GetWidth();
	m_MainViewport.Height = (float)g_SceneColorBuffer.GetHeight();
	m_MainViewport.MinDepth = 0.0f;
	m_MainViewport.MaxDepth = 1.0f;

	m_MainScissor.left = 0;
	m_MainScissor.top = 0;
	m_MainScissor.right = (LONG)g_SceneColorBuffer.GetWidth();
	m_MainScissor.bottom = (LONG)g_SceneColorBuffer.GetHeight();
}

void BakedGI::RenderObjects(GraphicsContext& gfxContext, Matrix4 viewProjMatrix, eObjectFilter filter)
{
	struct VSConstants
	{
		//Matrix4 viewMatrix;
		//Matrix4 projMatrix;
		Matrix4 viewProjMatrix;
		//Matrix4 clusterMatrix;
		//Vector4 screenParam;
		//Vector4 projectionParam;
	} perCameraConstants;
	perCameraConstants.viewProjMatrix = viewProjMatrix;

	gfxContext.SetDynamicConstantBufferView(0, sizeof(perCameraConstants), &perCameraConstants);

	uint32_t materialIdx = 0xFFFFFFFFul;

	uint32_t VertexStride = m_Model.m_VertexStride;

	for (uint32_t meshIndex = 0; meshIndex < m_Model.m_Header.meshCount; meshIndex++)
	{
		const Model::Mesh& mesh = m_Model.m_pMesh[meshIndex];

		uint32_t indexCount = mesh.indexCount;
		uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
		uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

		if (mesh.materialIndex != materialIdx)
		{
			if (m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kCutout) ||
				!m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kOpaque))
				continue;

			materialIdx = mesh.materialIndex;
			gfxContext.SetDynamicDescriptors(2, 0, 6, m_Model.GetSRVs(materialIdx));
		}

		gfxContext.SetConstants(4, baseVertex, materialIdx);
		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
	}
}

void BakedGI::RenderScene( void )
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

	{
		ScopedTimer _prof(L"Render Color", gfxContext);

		// Set the default state for command lists
		auto& pfnSetupGraphicsState = [&](void)
		{
			gfxContext.SetRootSignature(m_RootSig);
			gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
			gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());
		};

		pfnSetupGraphicsState();

		gfxContext.SetPipelineState(m_ModelPSO);
		gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		gfxContext.ClearColor(g_SceneColorBuffer);
		gfxContext.ClearDepth(g_SceneDepthBuffer);
		gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
		gfxContext.SetViewportAndScissor(m_MainViewport, m_MainScissor);

		RenderObjects(gfxContext, m_ViewProjMatrix, kOpaque);
	}

    // Rendering something

    gfxContext.Finish();
}
