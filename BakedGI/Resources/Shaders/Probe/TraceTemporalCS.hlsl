#include "ProbeRS.hlsli"
#include "../Utils/Octahedral.hlsli"

cbuffer ProbeParams : register(b1)
{
    float4x4 viewProjMatrix;
    float4x4 viewProjMatrixLastFrame;
};

RWTexture2D<float3> traceResult : register(u0);
RWTexture2D<float3> traceResultLastFrame : register(u1);
RWTexture2D<float3> finalColorBuffer : register(u2);

Texture2D<float> depthBuffer : register(t0);

SamplerState sampler0 : register(s0);

[RootSignature(Temporal_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

}