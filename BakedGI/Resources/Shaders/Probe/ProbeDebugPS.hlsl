
#include "../Utils/Resouces.hlsli"
#include "../Utils/Octahedral.hlsli"

cbuffer ProbeParams : register(b0)
{
    float4 probeRes;
    float4 screenParams;
    float4 probeParams;
    float4 probePosMin;
    float4 probePosMax;
    float4 probeZParam;
};

Texture2DArray<half3> irradianceMap : register(t0);
Texture2DArray<half2> normalMap : register(t1);
Texture2DArray<half> distMap : register(t2);


TextureCubeArray<float3> irradianceCubeMap : register(t32);
TextureCubeArray<float2> normalCubeMap : register(t33);
TextureCubeArray<float> distanceCubeMap : register(t34);

SamplerState sampler0 : register(s0);

struct VertexOutputProbe
{
    float4 clipPos : SV_Position;
    float3 cubeUV : TexCoord0;
};

[RootSignature(Probe_RootSig)]
float3 main(VertexOutputProbe i) : SV_Target0
{    
    float3 dir = i.cubeUV;
    dir.y *= -1;
    float2 uv = octEncode(dir);
    uv = uv * 0.5 + 0.5;
    float3 finalColor = 0;
    finalColor = irradianceMap.SampleLevel(sampler0, float3(uv, probeParams.w), 0);
    return finalColor;
}