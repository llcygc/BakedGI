#pragma once

#include "pch.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "ShadowManager.h"
#include "../Scene/Scene.h"

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
	Matrix4 viewProjMatrix;
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

	void Initialize();
	void AddLocalLight(LightData l, bool clear = true);
	void SetDirectionalLight(LightData l);
	void PrepareLightsDataForGPU(GraphicsContext& gfxContext, uint32_t rootIndex); 
	void PrepareLightsDataForGPUCS(ComputeContext& csContext, uint32_t rootIndex);
	void ClusterLightAssignment(GraphicsContext& gfxContext);
	void RenderShadows(GraphicsContext& gfxContext, Scene& scene);
	void Release();

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

