cbuffer PerCamera : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 viewProjMatrix;
	int directionLightCount;
};

float4 PositionObjectToWorld(float4 objectPos)
{
	return objectPos;
}

float4 PositionWorldToClip(float4 worldPos)
{
	return worldPos;
}

float3 DirectionObjectToWorld(float3 dir)
{
	return dir;
}