#include "DeferredLightingRS.hlsli"
#include "../Utils/Octahedral.hlsli"
#include "Lighting.hlsli"

cbuffer PerCamera : register(b1)
{
	//float4x4 viewMatrix;
	//float4x4 projMatrix;
	float4x4 InvViewProjMatrix;
	float4 ScreenParams;
	float3 CameraPos;

	//float4x4 clusterMatrix;
	//float4 screenParam;
	//float4 projectionParam;
}; 

cbuffer DirectionalLight : register(b2)
{
	float4x4 dirShadowMatrix;
	float4 positionType;
	float4 colorAngle;
	float4 forwardRange;
	float4 shadowParams;
}

Texture2D<half4> GBuffer0 : register(t0);
Texture2D<half4> GBuffer1 : register(t1);
Texture2D<half4> GBuffer2 : register(t2);
Texture2D<float> DepthBuffer : register(t3);
RWTexture2D<float3> OutColor : register(u0);

Texture2D<float> texShadow : register(t64);
SamplerComparisonState shadowSampler : register(s0);

float GetShadow(float3 worldPos)
{
	float3 ShadowCoord = mul(dirShadowMatrix, float4(worldPos, 1.0)).xyz;
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

[RootSignature(Deferred_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 diffuse = GBuffer0[DTid.xy];
    float4 normalData = GBuffer1[DTid.xy];
    float4 mso = GBuffer2[DTid.xy];
    float depth = DepthBuffer[DTid.xy];

	float2 normalOct24 = unorm8x3_to_snorm12x2(normalData.rgb);
	float3 normalWorld = octDecode(normalOct24);

	float2 screenPos = ((float2)DTid.xy + 0.5) / ScreenParams.xy;
	screenPos = screenPos * 2.0f - 1.0f;
	float4 clipPos = float4(screenPos.x, -screenPos.y, depth, 1.0f);
	float4 posWorld = mul(InvViewProjMatrix, clipPos);
	posWorld /= posWorld.w;

	BRDFData brdfData;
	brdfData.diffuse = half4(diffuse.rgb, 1.0h);
	brdfData.glossness = mso.g;
	brdfData.metallic = mso.r;

	PositionData posData;
	posData.normalWorld = normalWorld;
	posData.posWorld = posWorld;
	posData.viewDiretion = normalize(CameraPos - posWorld);

	float3 finalColor = DirectLighting(posData, brdfData, colorAngle.rgb * GetShadow(posWorld), forwardRange.xyz);

	OutColor[DTid.xy] = finalColor;// worldPos.xyz;// GBuffer0[DTid.xy].rgb;
}