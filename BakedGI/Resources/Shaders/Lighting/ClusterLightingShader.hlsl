//felix wang
//Standard Shader for Physically based Shading by Cluster Lighting

#include "../Utils/Basics.hlsli"

struct VertexInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 uv : TEXCOORD0;
};

struct VertexOutput
{
	float4 clipPos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 posWS : TEXCOORD1;
	float4 normalWS : TEXCOORD2;
	float4 tangentWS : TEXCOORD3;
	float4 binormalWS : TEXCOORD4;
	float4 posCluster : TEXCOORD5;
};

float4 Vert( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}