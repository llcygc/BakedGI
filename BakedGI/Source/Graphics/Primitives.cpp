#include "Primitives.h"



Primitives::Primitives()
{
}


Primitives::~Primitives()
{
	m_VertexBuffer.Destroy();
	m_IndexBuffer.Destroy();
}

void Primitives::RenderPrimitive(GraphicsContext& gfxContext, const Camera& cam, Matrix4 worldMat)
{
	gfxContext.SetIndexBuffer(m_IndexBuffer.IndexBufferView());
	gfxContext.SetVertexBuffer(0, m_VertexBuffer.VertexBufferView());

	struct VSConstants
	{
		//Matrix4 viewMatrix;
		//Matrix4 projMatrix;
		Matrix4 worldMatrix;
		Matrix4 viewProjMatrix;
		//Matrix4 clusterMatrix;
		//Vector4 screenParam;
		//Vector4 projectionParam;
		XMFLOAT3 cameraPos;
	} perCameraConstants;

	perCameraConstants.viewProjMatrix = cam.GetViewProjMatrix();
	perCameraConstants.worldMatrix = worldMat;
	XMStoreFloat3(&perCameraConstants.cameraPos, cam.GetPosition());

	gfxContext.SetDynamicConstantBufferView(0, sizeof(perCameraConstants), &perCameraConstants);

	gfxContext.DrawIndexed(INDEX_COUNT, 0, 0);
}

void Sphere::Create(void)
{
	int indexSize = 4;
	m_VertexStride = sizeof(Vertex);

	m_pVertexData = (void*)vertices;
	m_pIndexData = (void*)indices;

	VERTEX_COUNT = 515;
	INDEX_COUNT = 2304;

	m_VertexBuffer.Create(L"Sphere Vertices", VERTEX_COUNT, m_VertexStride, m_pVertexData);
	m_IndexBuffer.Create(L"Sphere Indices", INDEX_COUNT, indexSize, m_pIndexData);
}

Sphere::~Sphere()
{

}