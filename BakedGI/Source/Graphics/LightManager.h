#pragma once

#include "PipelineState.h"
#include "GpuBuffer.h"
#include "ShadowManager.h"

class LightManager
{
public:
	LightManager();
	~LightManager();

	void PrepareLightsDataForGPU();

public:
	ByteAddressBuffer m_StartOffsetIndexBuffer;
	StructuredBuffer m_LightLinkListBuffer;

	StructuredBuffer m_DirectionalLightsBuffer;
	StructuredBuffer m_PunctualLightsGeoBuffer;
	StructuredBuffer m_PunctualLightsRenderBuffer;

	ComputePSO m_ClusterLightAssignment_32;

private:
	ShadowManager m_ShadowManager;

	const int MAX_LOCAL_LIGHTS_COUNT = 20;
	const int MAX_SHADOW_CASTER_COUNT = 10;
	const int MAX_DIRECTIONAL_LIGHTS_COUNT = 4;
};

