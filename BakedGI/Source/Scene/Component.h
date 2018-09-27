#pragma once

enum ComponentType
{
	MESH,
	LIGHT
};

class Component
{
public:
	Component();
	~Component();

public:
	ComponentType type;
};

