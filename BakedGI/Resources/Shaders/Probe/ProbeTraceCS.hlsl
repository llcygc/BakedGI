#include "ProbeRS.hlsli"
#include "../Utils/Octahedral.hlsli"

Texture2DArray<float3> irradianceOctanMap : register(t0);
Texture2DArray<float2> normalOctanMap : register(t1);
Texture2DArray<float> distanceOctanMap : register(t2);
Texture2DArray<float> distanceMinMipOctaMap : register(t3);

RWTexture2D<float3> traceResult;

SamplerState sampler0 : register(s0);

[RootSignature(ProbeTrace_RootSig)]
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}