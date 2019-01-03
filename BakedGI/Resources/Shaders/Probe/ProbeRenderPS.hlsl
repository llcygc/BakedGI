#include "InputData.hlsli"

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

[RootSignature(ProbeRender_RootSig)]
PixelOutput main(VertexOutput i) : SV_TARGET
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

	float2 normalOct24 = octEncode(normal);
	float3 normalRGB8 = snorm12x2_to_unorm8x3(normalOct24);

	PixelOutput o;
	o.irradiance = brdfData.diffuse.rgb;
	o.normal = normalOct24;
	o.distance = length(i.posWS.xyz - cameraPos);

	return o;
}