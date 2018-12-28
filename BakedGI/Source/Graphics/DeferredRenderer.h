#pragma once

#include "pch.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "Model.h"
#include "Camera.h"
#include "BufferManager.h"

#include "CompiledShaders/GBufferShaderVS.h"
#include "CompiledShaders/GBufferShaderPS.h"

using namespace Graphics;

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

	void Initialize(const Model& model);
	void Update();
	void Render(GraphicsContext& gfxContext, const Model& model, const Camera& cam, D3D12_VIEWPORT viewport, D3D12_RECT scissor);
	void Release();

private:

	enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };

	RootSignature m_GBufferSig;
	RootSignature m_LitSig;

	ColorBuffer GBuffer0;
	ColorBuffer GBuffer1;
	ColorBuffer GBuffer2;

	GraphicsPSO m_GBufferPSO;
	GraphicsPSO m_GBufferCutoutPSO;
	ComputePSO DeferredLightingPSO;

	std::vector<bool> m_pMaterialIsCutout;

	void RenderObjects(GraphicsContext& gfxContext, const Model& model, const Camera& cam, eObjectFilter filter);
};

