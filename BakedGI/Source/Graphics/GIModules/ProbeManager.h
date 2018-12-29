#pragma once

#include "pch.h"
#include "VectorMath.h"

#include "GraphicsCore.h"
#include "GpuBuffer.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "BufferManager.h"

#include "CompiledShaders/ClusterLightingShaderVS.h"
#include "CompiledShaders/ClusterLightingShaderPS.h"

using namespace Math;
using namespace Graphics;

struct ProbeMap
{
	Vector3 position;
	Matrix4 cubeMatrices[6];
};

class ProbeManager
{
public:
	ProbeManager();
	~ProbeManager();

	void SetUpProbes(Vector3 min, Vector3 max, Vector3 division, int resolution = 1024);
	void RenderProbeMaps();
	void SetUpGpuDatas();

private:

	void ReprojCubetoOctan();

	uint32_t m_probeCount;
	Vector3 m_probeDimension;
	
	std::vector<ProbeMap> m_probeMaps;
	ColorBuffer m_irradianceMapOctan;
	ColorBuffer m_normalMapOctan;
	ColorBuffer m_distanceMapOctan;

	ColorBuffer m_irradianceMapCube;
	ColorBuffer m_normalMapCube;
	ColorBuffer m_distanceMapCube;

	DepthBuffer m_depthBuffer;
	StructuredBuffer m_ProbeMatrixBuffer;

	GraphicsPSO m_debugDisplayPSO;
	GraphicsPSO m_probeRenderPSO;
	ComputePSO m_CubeToOctanPSO;

	RootSignature m_debugDisplayRootSig;
	RootSignature m_probeRenderRootSig;
};

