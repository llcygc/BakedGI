#include "ProbeRS.hlsli"

cbuffer ProbeParams : register(b1)
{
    float4 probeRes;
    float4 screenParams;
};

TextureCubeArray<float3> irradianceCubeMap : register(t0);
TextureCubeArray<float2> normalCubeMap : register(t1);
TextureCubeArray<float> distanceCubeMap : register(t2);

RWTexture2DArray<float3> irradianceOctanMap : register(u0);
RWTexture2DArray<float2> normalOctanMap : register(u1);
RWTexture2DArray<float> distanceOctanMap : register(u2);
RWTexture2DArray<float> distanceMinMipOctaMap : register(u3);

SamplerState sampler0 : register(s0);

groupshared float minDist = 10000.0f;

[RootSignature(CubeToOctan_RootSig)]
[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    
    float dist = distanceOctanMap[DTid];

    if (minDist > dist)
        minDist = dist;

    GroupMemoryBarrierWithGroupSync();

    if (DTid.x == 0 && DTid.y == 0)
        distanceMinMipOctaMap[DTid] = minDist;
}