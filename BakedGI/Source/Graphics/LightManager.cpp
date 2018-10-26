#include "LightManager.h"
#include "CommandContext.h"


LightManager::LightManager()
{
}


LightManager::~LightManager()
{
	m_LocalLights.clear();
}

void LightManager::SetDirectionalLight(Light l)
{
	if(l.positionType.GetW() == (float)LightType::Directional)
		m_DirectionalLight = l;
}

void LightManager::AddLocalLight(Light l, bool clear)
{
	if (clear)
		m_LocalLights.clear();
	m_LocalLights.push_back(l);
}

void LightManager::PrepareLightsDataForGPU()
{

}

void LightManager::ClusterLightAssignment(GraphicsContext gfxContext)
{

}
