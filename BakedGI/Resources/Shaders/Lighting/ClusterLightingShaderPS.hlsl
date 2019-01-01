
#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"
#include "InputData.hlsli"
#include "Lighting.hlsli"

cbuffer DirectionalLight : register(b0)
{
    float4x4 dirShadowMatrix;
    float4x4 shadowViewProjMatrix;
    float4 positionType;
    float4 colorAngle;
    float4 forwardRange;
    float4 shadowParams;
}

Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texSpecular : register(t1);
//Texture2D<float4> texEmissive		: register(t2);
Texture2D<float3> texNormal : register(t3);

Texture2D<float> texShadow : register(t64);

//StructuredBuffer<LocalLightData> lightBuffer : register(t66);
//Texture2DArray<float> lightShadowArrayTex : register(t67);
//ByteAddressBuffer lightGrid : register(t68);
//ByteAddressBuffer lightGridBitMask : register(t69);

SamplerState sampler0 : register(s0);
SamplerComparisonState shadowSampler : register(s1);

float GetShadow(float3 worldPos)
{    
    float3 ShadowCoord = mul(dirShadowMatrix, float4(worldPos, 1.0)).xyz;
    ShadowCoord.z += 0.1;
    const float Dilation = 2.0;
    float d1 = Dilation * shadowParams.z * 0.125;
    float d2 = Dilation * shadowParams.z * 0.875;
    float d3 = Dilation * shadowParams.z * 0.625;
    float d4 = Dilation * shadowParams.z * 0.375;
    float result = (
        2.0 * texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy, ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d2, d1), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d1, -d2), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d2, -d1), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d1, d2), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d4, d3), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(-d3, -d4), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d4, -d3), ShadowCoord.z) +
        texShadow.SampleCmpLevelZero(shadowSampler, ShadowCoord.xy + float2(d3, d4), ShadowCoord.z)
        ) / 10.0;
    return result * result;
}

[RootSignature(Standard_RootSig)]
float3 main(VertexOutput i) : SV_Target0
{
    uint2 pixelPos = i.clipPos.xy;
    float3 diffuseAlbedo = texDiffuse.Sample(sampler0, i.uv);
    float3 colorSum = 0;
    {
        //float ao = texSSAO[pixelPos];
        //colorSum += ApplyAmbientLight(diffuseAlbedo, ao, AmbientColor);
    }

    float gloss = 1.0;
    float3 normal;
    {
        normal = texNormal.Sample(sampler0, i.uv) * 2.0 - 1.0;
        //AntiAliasSpecular(normal, gloss);
        float3x3 tbn = float3x3(normalize(i.tangentWS), normalize(i.binormalWS), normalize(i.normalWS));
        normal = normalize(mul(normal, tbn));
    }
    
    float glossness = texSpecular.Sample(sampler0, i.uv) * gloss;
    BRDFData brdfData;
    brdfData.diffuse = half4(diffuseAlbedo.rgb, 1.0h);
    half3 mso = clamp(texSpecular.Sample(sampler0, i.uv), 0.0f, 0.99f);
    brdfData.glossness = mso.g;
    brdfData.metallic = mso.r;

    PositionData posData;
    posData.normalWorld = normal;
    posData.posWorld = i.posWS;
    posData.viewDiretion = normalize(i.viewDir);

    colorSum += DirectLighting(posData, brdfData, colorAngle.rgb * GetShadow(i.posWS.xyz), forwardRange.xyz);

    return colorSum;
}