#pragma once

#include <vector>
#include "Scene.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	static SceneManager* GetInstance()
	{
		if (!instance)
			instance = new SceneManager();

		return instance;
	}

	Scene SetupScene(const char* fileName);

private:
	static SceneManager* instance;
};

