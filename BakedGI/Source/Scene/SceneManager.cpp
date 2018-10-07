#include "SceneManager.h"



SceneManager::SceneManager()
{
}


SceneManager::~SceneManager()
{
}

Scene* SetupScene(const char* fileName)
{
	return new Scene();
}
