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

//struct Light
//{
//	Vector4 position;
//	Vector4 color;
//	Quaternion rotation;
//	float range;
//	float intensity;
//	float spotAngle;
//	int shadowIndex;
//	LightType type;
//};

struct LightData
{
	Matrix4 worldToShadowMatrix;
	Vector4 positionType;
	Vector4 colorAngle;
	Vector4 forwardRange;
	Vector4 shadowParams;
};

class LightManager
{
public:
	LightManager();
	~LightManager();

	void AddLocalLight(LightData l, bool clear = true);
	void SetDirectionalLight(LightData l);
	void PrepareLightsDataForGPU(GraphicsContext& gfxContext);
	void ClusterLightAssignment(GraphicsContext gfxContext);

public:

private:
	static const int MAX_LOCAL_LIGHTS_COUNT = 64;
	static const int MAX_SHADOW_CASTER_COUNT = 10;
	static const int MAX_DIRECTIONAL_LIGHTS_COUNT = 2;

	ByteAddressBuffer m_StartOffsetIndexBuffer;
	StructuredBuffer m_LightLinkListBuffer;

	StructuredBuffer m_DirectionalLightsBuffer;
	StructuredBuffer m_PunctualLightsGeoBuffer;
	StructuredBuffer m_PunctualLightsRenderBuffer;

	ComputePSO m_ClusterLightAssignment_32;

	ShadowManager m_ShadowManager;
	std::vector<LightData> m_LocalLights;
	//Light m_LocalLights[MAX_LOCAL_LIGHTS_COUNT];
	LightData m_DirectionalLight;
};

