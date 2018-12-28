
#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"
#include "InputData.hlsli"
#include "BRDF.hlsli"
#include "LightData.hlsli"
#include "../Utils/Octahedral.hlsli"

Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texSpecular : register(t1);
//Texture2D<float4> texEmissive		: register(t2);
Texture2D<float3> texNormal : register(t3);

SamplerState sampler0 : register(s0);

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

	return colorSum;
}