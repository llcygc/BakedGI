#pragma once

#include "PipelineState.h"
#include "GpuBuffer.h"

class LightManager
{
public:
	LightManager();
	~LightManager();

public:
	ByteAddressBuffer m_StartOffsetIndexBuffer;
	StructuredBuffer m_LightLinkListBuffer;

	StructuredBuffer m_DirectionalLightsBuffer;
	StructuredBuffer m_PunctualLightsGeoBuffer;
	StructuredBuffer m_PunctualLightsRenderBuffer;

	ComputePSO m_ClusterLightAssignment_32;

};

