#include "Scene.h"



Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::Initialize(std::string modelPath)
{
	ASSERT(m_Model.Load((ResourceManager::GetResourceRootPath() + modelPath).c_str()));
	ASSERT(m_Model.m_Header.meshCount > 0, "Model has no mesh in it");

	m_pMaterialIsCutout.resize(m_Model.m_Header.materialCount);
	for (uint32_t i = 0; i < m_Model.m_Header.materialCount; ++i)
	{
		const Model::Material& mat = m_Model.m_pMaterial[i];
		if (std::string(mat.texDiffusePath).find("thorn") != std::string::npos ||
			std::string(mat.texDiffusePath).find("plant") != std::string::npos ||
			std::string(mat.texDiffusePath).find("chain") != std::string::npos)
		{
			m_pMaterialIsCutout[i] = true;
		}
		else
		{
			m_pMaterialIsCutout[i] = false;
		}
	}

	float modelRadius = Length(m_Model.m_Header.boundingBox.max - m_Model.m_Header.boundingBox.min) * .5f;
	const Vector3 eye = (m_Model.m_Header.boundingBox.min + m_Model.m_Header.boundingBox.max) * .5f + Vector3(modelRadius * .5f, 0.0f, 0.0f);
	m_Camera.SetEyeAtUp(eye, Vector3(kZero), Vector3(kYUnitVector));
	m_Camera.SetZRange(1.0f, 10000.0f);
	m_CameraController.reset(new CameraController(m_Camera, Vector3(kYUnitVector)));
}

void Scene::Update(float deltaT)
{
	m_CameraController->Update(deltaT);
}

void Scene::RenderScene(GraphicsContext& gfxContext, eObjectFilter filter)
{
	gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
	gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());

	struct VSConstants
	{
		//Matrix4 viewMatrix;
		//Matrix4 projMatrix;
		Matrix4 objectToWorldMatrix;
		Matrix4 viewProjMatrix;
		//Matrix4 clusterMatrix;
		//Vector4 screenParam;
		//Vector4 projectionParam;
		XMFLOAT3 cameraPos;
	} perCameraConstants;

	perCameraConstants.viewProjMatrix = m_Camera.GetViewProjMatrix();
	perCameraConstants.objectToWorldMatrix = Matrix4(kIdentity);
	XMStoreFloat3(&perCameraConstants.cameraPos, m_Camera.GetPosition());

	gfxContext.SetDynamicConstantBufferView(1, sizeof(perCameraConstants), &perCameraConstants);

	uint32_t materialIdx = 0xFFFFFFFFul;

	uint32_t VertexStride = m_Model.m_VertexStride;

	for (uint32_t meshIndex = 0; meshIndex < m_Model.m_Header.meshCount; meshIndex++)
	{
		const Model::Mesh& mesh = m_Model.m_pMesh[meshIndex];

		uint32_t indexCount = mesh.indexCount;
		uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
		uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

		if (mesh.materialIndex != materialIdx)
		{
			if (m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kCutout) ||
				!m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kOpaque))
				continue;

			materialIdx = mesh.materialIndex;
			gfxContext.SetDynamicDescriptors(2, 0, 6, m_Model.GetSRVs(materialIdx));
		}

		gfxContext.SetConstants(0, baseVertex, materialIdx);
		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
	}
}

void Scene::RenderScene(GraphicsContext& gfxContext, Matrix4 ViewProjMatrix, eObjectFilter filter)
{
	gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
	gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());

	struct VSConstants
	{
		//Matrix4 viewMatrix;
		//Matrix4 projMatrix;
		Matrix4 objectToWorldMatrix;
		Matrix4 viewProjMatrix;
		//Matrix4 clusterMatrix;
		//Vector4 screenParam;
		//Vector4 projectionParam;
		XMFLOAT3 cameraPos;
	} perCameraConstants;

	perCameraConstants.viewProjMatrix = ViewProjMatrix;
	perCameraConstants.objectToWorldMatrix = Matrix4(kIdentity);
	XMStoreFloat3(&perCameraConstants.cameraPos, m_Camera.GetPosition());

	gfxContext.SetDynamicConstantBufferView(1, sizeof(perCameraConstants), &perCameraConstants);

	uint32_t materialIdx = 0xFFFFFFFFul;

	uint32_t VertexStride = m_Model.m_VertexStride;

	for (uint32_t meshIndex = 0; meshIndex < m_Model.m_Header.meshCount; meshIndex++)
	{
		const Model::Mesh& mesh = m_Model.m_pMesh[meshIndex];

		uint32_t indexCount = mesh.indexCount;
		uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
		uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

		if (mesh.materialIndex != materialIdx)
		{
			if (m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kCutout) ||
				!m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kOpaque))
				continue;

			materialIdx = mesh.materialIndex;
			gfxContext.SetDynamicDescriptors(2, 0, 6, m_Model.GetSRVs(materialIdx));
		}

		gfxContext.SetConstants(0, baseVertex, materialIdx);
		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
	}
}

void Scene::RenderScene(GraphicsContext& gfxContext, const Camera& cam, eObjectFilter filter)
{
	gfxContext.SetIndexBuffer(m_Model.m_IndexBuffer.IndexBufferView());
	gfxContext.SetVertexBuffer(0, m_Model.m_VertexBuffer.VertexBufferView());

	struct VSConstants
	{
		//Matrix4 viewMatrix;
		//Matrix4 projMatrix;
		Matrix4 objectToWorldMatrix;
		Matrix4 viewProjMatrix;
		//Matrix4 clusterMatrix;
		//Vector4 screenParam;
		//Vector4 projectionParam;
		XMFLOAT3 cameraPos;
	} perCameraConstants;

	perCameraConstants.viewProjMatrix = cam.GetViewProjMatrix();
	perCameraConstants.objectToWorldMatrix = Matrix4(kIdentity);
	XMStoreFloat3(&perCameraConstants.cameraPos, cam.GetPosition());

	gfxContext.SetDynamicConstantBufferView(1, sizeof(perCameraConstants), &perCameraConstants);

	uint32_t materialIdx = 0xFFFFFFFFul;

	uint32_t VertexStride = m_Model.m_VertexStride;

	for (uint32_t meshIndex = 0; meshIndex < m_Model.m_Header.meshCount; meshIndex++)
	{
		const Model::Mesh& mesh = m_Model.m_pMesh[meshIndex];

		uint32_t indexCount = mesh.indexCount;
		uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
		uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

		if (mesh.materialIndex != materialIdx)
		{
			if (m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kCutout) ||
				!m_pMaterialIsCutout[mesh.materialIndex] && !(filter & kOpaque))
				continue;

			materialIdx = mesh.materialIndex;
			gfxContext.SetDynamicDescriptors(2, 0, 6, m_Model.GetSRVs(materialIdx));
		}

		gfxContext.SetConstants(0, baseVertex, materialIdx);
		gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
	}
}