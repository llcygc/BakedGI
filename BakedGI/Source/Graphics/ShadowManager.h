#pragma once

#include "pch.h"
#include "GraphicsCore.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "BufferManager.h"
#include "RootSignature.h"
//#include "LightManager.h"
#include "../Scene/Scene.h"

#include "CompiledShaders/DepthShaderVS.h"
#include "CompiledShaders/DepthShaderPS.h"

using namespace Math;
using namespace Graphics;

struct ShadowData
{
	Vector4 ShadowParams;
	Matrix4 ShadowMatrix;
};

class ShadowManager
{
public:
	ShadowManager();
	~ShadowManager();

	void Initialize();
	void Render(GraphicsContext& gfxContext, Scene& scene, Matrix4 shadowMatrix);

private:
	GraphicsPSO m_ShadowPSO;
	GraphicsPSO m_CutoutShadowPSO;

	RootSignature m_ShadowRootSig;
};

