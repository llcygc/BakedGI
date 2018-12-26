#pragma once

#include "pch.h"
#include "GraphicsCore.h"
#include "ColorBuffer.h"
#include "CompiledShaders/GBufferShaderVS.h"
#include "CompiledShaders/GBufferShaderPS.h"

class DeferredRenderer
{
public:
	DeferredRenderer();
	~DeferredRenderer();

	void Initialize(uint32_t width, uint32_t height);
	void Update();
	void Render();
	void Release();

public:
	ColorBuffer GBuffer0;
	ColorBuffer GBuffer1;
	ColorBuffer GBuffer2;

	GraphicsPSO
};

