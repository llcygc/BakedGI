#pragma once

#include "pch.h"
#include "PipelineState.h"
#include "GpuBuffer.h"

using namespace Math;

struct ShadowData
{
	Vector4 ShadowParams;
	Matrix4 ShadowMatrix;
};

class ShadowManager
{
public:
	ShadowManager();
	~ShadowManager();
};

