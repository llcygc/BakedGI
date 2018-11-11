#include "LightManager.h"
#include "CommandContext.h"


LightManager::LightManager()
{
}


LightManager::~LightManager()
{
	m_LocalLights.clear();
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

void LightManager::PrepareLightsDataForGPU(GraphicsContext& gfxContext)
{
	gfxContext.SetDynamicConstantBufferView(1, sizeof(LightData), &m_DirectionalLight);
}

void LightManager::ClusterLightAssignment(GraphicsContext gfxContext)
{

}
