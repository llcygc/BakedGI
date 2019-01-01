#include "ProbeRS.hlsli"
#include "../Utils/Octahedral.hlsli"

cbuffer ProbeParams : register(b1)
{
    float4x4 viewProjMatrix;
    float4x4 viewProjMatrixLastFrame;
};

RWTexture2D<float3> traceResult : register(u0);

Texture2D<float> depthBuffer : register(t0);
Texture2D<float3> traceResultLastFrame : register(t1);

SamplerState sampler0 : register(s0);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

}