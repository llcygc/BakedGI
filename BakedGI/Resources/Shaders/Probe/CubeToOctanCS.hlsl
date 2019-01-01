#include "ProbeRS.hlsli"
#include "../Utils/Octahedral.hlsli"

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

[RootSignature(CubeToOctan_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

}