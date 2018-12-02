#include "Primitives.h"



Primitives::Primitives()
{
}


Primitives::~Primitives()
{
}

void Sphere::Create(void)
{
	int indexSize = 4;
	m_VertexStride = sizeof(VertexData);

	m_pVertexData = (void*)vertexData;
	m_pIndexData = (void*)indices;

	m_VertexBuffer.Create(L"Sphere Vertices", VERTEX_COUNT, m_VertexStride, m_pVertexData);
	m_IndexBuffer.Create(L"Sphere Indices", INDEX_COUNT, indexSize, m_pIndexData);
}