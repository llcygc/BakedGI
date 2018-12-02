
#include "../Utils/Resouces.hlsli"

cbuffer ProbeInfo : register(b0)
{
    float4 scaleOffset;
    float4 resolution;
}

Texture2D<half3> irradianceMap : register(t0);
Texture2D<half3> normalMap : register(t1);
Texture2D<half> distMap : register(t2);

SamplerState sampler0 : register(s0);

struct VertexOutputProbe
{
    float4 clipPos : SV_Position;
};

[RootSignature(Probe_RootSig)]
float3 main(VertexOutputProbe i) : SV_Target0
{
	return float3(1.0f, 0.0f, 0.0f);
}