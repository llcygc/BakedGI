#pragma once

#include "pch.h"
#include "VectorMath.h"

#include "GpuBuffer.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"

using namespace Math;

struct ProbeMap
{
	Vector4 position;
	Matrix4 cubeMatrices[6];
	ColorBuffer irradianceMap;
	ColorBuffer normalMap;
	ColorBuffer distance;
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

	DepthBuffer m_depthBuffer;

	StructuredBuffer m_ProbeMatrixBuffer;
};

