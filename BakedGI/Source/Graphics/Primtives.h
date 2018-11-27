#pragma once

#include "VectorMath.h"
#include "GpuBuffer.h"

using namespace Math;

class Primtives
{
public:
	Primtives();
	~Primtives();

	virtual void Create(void) = 0;

	unsigned char* m_pVertexData;
	unsigned char* m_pIndexData;
	StructuredBuffer m_VertexBuffer;
	ByteAddressBuffer m_IndexBuffer;
	uint32_t m_VertexStride;
};

class Sphere : public Primtives
{
	virtual void Create(void) override;
};

class Cube : public Primtives
{
	virtual void Create(void) override;
};

class Capsule : public Primtives
{
	virtual void Create(void) override;
};

class Quad : public Primtives
{
	virtual void Create(void) override;
};