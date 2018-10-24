#pragma once

#include "pch.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "ShadowManager.h"

using namespace Math;

enum LightType
{
	Directional,
	Spot,
	Point
};

struct Light
{
	Vector3 position;
	Vector3 color;
	Quaternion rotation;
	float range;
	float intensity;
	float spotAngle;
	bool enableShadow;
	LightType type;
};

class LightManager
{
public:
	LightManager();
	~LightManager();

	void AddLocalLight(Light l);
	void SetDirectionalLight(Light l);
	void PrepareLightsDataForGPU();
	void ClusterLightAssignment(GraphicsContext gfxContext);

public:
	ByteAddressBuffer m_StartOffsetIndexBuffer;
	StructuredBuffer m_LightLinkListBuffer; 

	StructuredBuffer m_DirectionalLightsBuffer;
	StructuredBuffer m_PunctualLightsGeoBuffer;
	StructuredBuffer m_PunctualLightsRenderBuffer;

	ComputePSO m_ClusterLightAssignment_32;

private:
	static const int MAX_LOCAL_LIGHTS_COUNT = 64;
	static const int MAX_SHADOW_CASTER_COUNT = 10;
	static const int MAX_DIRECTIONAL_LIGHTS_COUNT = 2;

	ShadowManager m_ShadowManager;
	Light m_LocalLights[MAX_LOCAL_LIGHTS_COUNT];
	Light m_DirectionalLight;
};

