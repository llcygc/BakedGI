#include "LightManager.h"
#include "CommandContext.h"


LightManager::LightManager()
{
}


LightManager::~LightManager()
{
}

void LightManager::Initialize()
{
	m_ShadowManager.Initialize();
}

void LightManager::SetDirectionalLight(LightData l)
{
	m_DirectionalLight = l;
}

void LightManager::AddLocalLight(LightData l, bool clear)
{
	if (clear)
		m_LocalLights.clear();
	m_LocalLights.push_back(l);
}

void LightManager::PrepareLightsDataForGPU(GraphicsContext& gfxContext, uint32_t rootIndex)
{
	gfxContext.SetDynamicConstantBufferView(rootIndex, sizeof(LightData), &m_DirectionalLight);
}

void LightManager::PrepareLightsDataForGPUCS(ComputeContext& csContext, uint32_t rootIndex)
{
	csContext.SetDynamicConstantBufferView(rootIndex, sizeof(LightData), &m_DirectionalLight);
}

void LightManager::ClusterLightAssignment(GraphicsContext& gfxContext)
{

}

void LightManager::RenderShadows(GraphicsContext& gfxContext, Scene& scene)
{
	m_ShadowManager.Render(gfxContext, scene, m_DirectionalLight.worldToShadowMatrix);
}

void LightManager::Release()
{
	m_LocalLights.clear();
}
