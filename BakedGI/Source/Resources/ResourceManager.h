#pragma once

#include "../../pch.h"
#include <stdio.h>

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	static std::wstring GetResourceRootPathWide()
	{
		return L"Resources/";
	}

	static std::string GetResourceRootPath()
	{
		return "Resources/";
	}

	static void* LoadShader(std::string shaderPath);
};

