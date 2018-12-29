#pragma once

#include "pch.h"
#include "Model.h"
#include "Camera.h"
#include "CameraController.h"
#include "GraphicsCore.h"
#include "BufferManager.h"
#include "CommandContext.h"

#include "../Resources/ResourceManager.h"

using namespace GameCore;

enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };

class Scene
{
public:
	Scene();
	~Scene();

	Camera m_Camera;
	std::auto_ptr<CameraController> m_CameraController;

	//Load model and texture resouces
	void Initialize(std::string modelPath);
	void Update(float detalT);
	void RenderScene(GraphicsContext& gfxContext, eObjectFilter filter);
	void RenderScene(GraphicsContext& gfxContext, Matrix4 vieProjMatrix, eObjectFilter filter);

	const Model::BoundingBox& GetSceneBoundingBox() { return m_Model.GetBoundingBox(); }

private:
	Model m_Model;
	std::vector<bool> m_pMaterialIsCutout;
};

