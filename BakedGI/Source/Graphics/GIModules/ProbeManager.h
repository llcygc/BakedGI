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
#include "../LightManager.h"
#include "../../Scene/Scene.h"

#include "CompiledShaders/ProbeRenderVS.h"
#include "CompiledShaders/ProbeRenderPS.h"
#include "CompiledShaders/ProbeDebugVS.h"
#include "CompiledShaders/ProbeDebugPS.h"
#include "CompiledShaders/CubeToOctanCS.h"
#include "CompiledShaders/MinMipCS.h"
#include "CompiledShaders/ProbeTraceCS.h"
#include "CompiledShaders/TraceTemporalCS.h"

using namespace Math;
using namespace Graphics;

struct Probe
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
	void RenderProbes(GraphicsContext& gfxContext, Scene& scene, D3D12_VIEWPORT viewport, D3D12_RECT scissor, LightManager& lightManger);
	void ComputeTrace(ComputeContext& cc, ColorBuffer& GBuffer0, ColorBuffer& GBuffer1, ColorBuffer& GBuffer2);
	void SetUpGpuDatas();
	void Release();

private:

	void ReprojCubetoOctan();
	D3D12_CPU_DESCRIPTOR_HANDLE CalSubRTV(ColorBuffer& destBuffer, int probeID, int faceID);
	void CreateCubemapResouceViews();

	Quaternion* cubeCamRotations;
	uint32_t m_probeCount;
	Vector3 m_probeDimension;
	uint32_t m_probeResolution;
	const uint32_t MINMIP_SIZE = 16;

	Camera m_probeCamera;
	
	std::vector<Probe> m_probes;
	ColorBuffer m_irradianceMapOctan;
	ColorBuffer m_normalMapOctan;
	ColorBuffer m_distanceMapOctan;
	ColorBuffer m_distanceMinMipMapOctan;

	ColorBuffer m_irradianceMapCube;
	ColorBuffer m_normalMapCube;
	ColorBuffer m_distanceMapCube;

	D3D12_CPU_DESCRIPTOR_HANDLE irradianceCubeRTVs[6];
	D3D12_CPU_DESCRIPTOR_HANDLE normalCubeRTVs[6];
	D3D12_CPU_DESCRIPTOR_HANDLE distanceCubeRTVs[6];

	D3D12_CPU_DESCRIPTOR_HANDLE irradianceCubeSRV;
	D3D12_CPU_DESCRIPTOR_HANDLE normalCubeSRV;
	D3D12_CPU_DESCRIPTOR_HANDLE distanceCubeSRV;

	DepthBuffer m_depthBuffer;
	StructuredBuffer m_ProbeMatrixBuffer;

	GraphicsPSO m_debugDisplayPSO;
	GraphicsPSO m_probeRenderPSO;
	ComputePSO m_cubeToOctanPSO;
	ComputePSO m_computeMinMipPSO;
	ComputePSO m_computeTracePSO;
	ComputePSO m_tracTemporalPSO;

	RootSignature m_debugDisplayRootSig;
	RootSignature m_probeRenderRootSig;
	RootSignature m_cubeToOctanProjRootSig;
	RootSignature m_computeTraceRootSig;
	RootSignature m_traceTemporalRootSig;
};

