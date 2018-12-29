#pragma once

#include "pch.h"
#include "GraphicsCore.h"
#include "CommandContext.h"
#include "Model.h"
#include "Camera.h"
#include "BufferManager.h"
#include "LightManager.h"
#include "../Scene/Scene.h"

#include "CompiledShaders/GBufferShaderVS.h"
#include "CompiledShaders/GBufferShaderPS.h"
#include "CompiledShaders/DeferredLightingCS.h"

using namespace Graphics;

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

	void Initialize();
	void Update();
	void Render(GraphicsContext& gfxContext, Scene& scene, D3D12_VIEWPORT viewport, D3D12_RECT scissor, LightManager& lightManger);
	void Release();

private:

	RootSignature m_GBufferSig;
	RootSignature m_LitSig;

	ColorBuffer GBuffer0;
	ColorBuffer GBuffer1;
	ColorBuffer GBuffer2;

	GraphicsPSO m_GBufferPSO;
	GraphicsPSO m_GBufferCutoutPSO;
	ComputePSO m_DeferredLightingPSO;
};

