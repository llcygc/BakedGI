#include "ProbeRS.hlsli"
#include "../Utils/Octahedral.hlsli"

cbuffer ProbeParams : register(b1)
{
    float4 probeRes;
    float4 screenParams;
    float4 probeParams;
    float4 probePosMin;
    float4 probePosMax;
    float4 probeZParam;
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
    
    uint probeIndex = DTid.z;

    float2 octanUV = float2(DTid.xy * probeRes.yy) * 2.0 - 1.0;
    float3 cubeUV = octDecode(octanUV);
    //cubeUV.y *= -1;
	
    irradianceOctanMap[DTid] = irradianceCubeMap.SampleLevel(sampler0, float4(cubeUV, probeIndex), 0); //SAMPLE_TEXTURECUBE_ARRAY_LOD(GI_ProbeTexture, sampler_GI_ProbeTexture, cubeUV, probeIndex, 0);
    normalOctanMap[DTid] = normalCubeMap.SampleLevel(sampler0, float4(cubeUV, probeIndex), 0); //SAMPLE_TEXTURECUBE_ARRAY_LOD(GI_DepthTexture, sampler_GI_DepthTexture, cubeUV, probeIndex, 0);
    distanceOctanMap[DTid] = distanceCubeMap.SampleLevel(sampler0, float4(cubeUV, probeIndex), 0); //SAMPLE_TEXTURECUBE_ARRAY_LOD(GI_NormalTexture, sampler_GI_NormalTexture, cubeUV, probeIndex, 0);

}