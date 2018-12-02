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
	int x = division.GetX();
	int y = division.GetY();
	int z = division.GetZ();

	m_probeCount = x * y * z;

	Vector3 dist = max - min;

	for(int i = 0; i < x; i++)
		for(int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
			{
				float xPos = (float)i / (x - 1);
				float yPos = (float)j / (y - 1);
				float zPos = (float)k / (z - 1);
				ProbeMap probe;
				probe.position = min + Vector3(xPos, yPos, zPos) * dist;
				m_probeMaps.push_back(probe);
			}

	SamplerDesc DefaultSamplerDesc;
	DefaultSamplerDesc.MaxAnisotropy = 8;

	m_debugDisplayRootSig.Reset(4, 1);
	m_debugDisplayRootSig.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_debugDisplayRootSig[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6, D3D12_SHADER_VISIBILITY_PIXEL);
	m_debugDisplayRootSig[3].InitAsConstants(1, 2, D3D12_SHADER_VISIBILITY_VERTEX);
	
	D3D12_INPUT_ELEMENT_DESC vertElem[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
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