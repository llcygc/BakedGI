#include "ResourceManager.h"



ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
}

void* ResourceManager::LoadShader(std::string fileName)
{
	FILE *shaderFile = nullptr;
	if (0 != fopen_s(&shaderFile, fileName.c_str(), "rb"))
		return nullptr;

	size_t shaderSize = sizeof(&shaderFile);
	if (shaderSize > 0)
	{
		void* shaderByteCode = malloc(shaderSize);
		if (1 != fread(shaderByteCode, shaderSize, 1, shaderFile))
			return shaderByteCode;
		else
			return nullptr;
	}
	else return nullptr;
}
