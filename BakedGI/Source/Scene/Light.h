#pragma once

#include "Component.h"
#include "../Core/Math/Vector.h"
#include "../Core/Math/Matrix4.h"

using namespace Math;

enum LightType
{
	DIRECTIONAL,
	SPOT,
	POINT,
	AREA
};

enum ShadowType
{
	NONE,
	PCF,
	EXP
};

class Light : public Component
{
public:
	Light();
	~Light();
	

public:
	LightType type;
	Vector3 position;
	Vector3 direction;
	ShadowType shadow;

	Matrix4 worldToLightMatrix;
	Matrix4 lightProjMatrix;
};

