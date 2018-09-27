#pragma once

#include <vector>

#include "../Core/Math/Vector.h"
#include "../Core//Math/Quaternion.h"
#include "../Core//Math/Matrix4.h"
#include "Component.h"

using namespace Math;

struct Transform
{
	Vector3 position;
	Vector3 scale;
	Quaternion rotation;

	Matrix4 localToWorldMatrix;
	Matrix4 worldToLocalMatrix;
};

class SceneNode
{
public:
	SceneNode();
	~SceneNode();

	Component* GetComponents()
	{
		return component;
	}
	
public:
	Transform transform;

private:
	Component* component = NULL;
	SceneNode* parent = NULL;
	std::vector<SceneNode*> children;
};

