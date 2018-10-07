#include "BRDF.hlsli"
#include "InputData.hlsli"

Texture2D albedoMap;
SamplerState sampler_albedoMap;

Texture2D normalMap;
SamplerState sampler_normalMap;

Texture2D mosMap;
SamplerState sampler_mosMap;

cbuffer PerMaterial : register(b1)
{

};

BRDFData SetupBRDFData(float2 uv)
{
    BRDFData data = (BRDFData) 0;

    return data;
}

PositionData SetupPositionData(VertexOutput i)
{
    PositionData data = (PositionData) 0;

    return data;
}