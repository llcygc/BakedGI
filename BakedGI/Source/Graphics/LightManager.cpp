#include "LightManager.h"



LightManager::LightManager()
{
}


LightManager::~LightManager()
{
}

void LightManager::SetDirectionalLight(Light l)
{
	if(l.type == LightType::Directional)
		m_DirectionalLight = l;
}

void LightManager::AddLocalLight(Light l)
{

}
