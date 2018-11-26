#include "ProbeManager.h"


ProbeManager::ProbeManager()
{
}


ProbeManager::~ProbeManager()
{
}

void ProbeManager::SetUpProbes(Vector3 min, Vector3 max, Vector3 division, int resolution)
{
	m_probeDimension = division;
	m_probeCount = division.GetX() * division.GetY() * division.GetZ();
}

void ProbeManager::RenderProbeMaps()
{

}

void ProbeManager::SetUpGpuDatas()
{

}

void ProbeManager::ReprojCubetoOctan()
{

}