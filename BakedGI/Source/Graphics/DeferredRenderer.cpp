#include "DeferredRenderer.h"


DeferredRenderer::DeferredRenderer()
{
}


DeferredRenderer::~DeferredRenderer()
{
}

void DeferredRenderer::Initialize(uint32_t width, uint32_t height)
{
	GBuffer0.Create(L"G-Buffer 0", width, height, 0, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS);
	GBuffer1.Create(L"G-Buffer 1", width, height, 0, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS);
	GBuffer2.Create(L"G-Buffer 2", width, height, 0, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS);
}

void DeferredRenderer::Update()
{

}

void DeferredRenderer::Render()
{

}

void DeferredRenderer::Release()
{
}
