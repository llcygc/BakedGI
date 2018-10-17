cbuffer PerCamera : register(b0)
{
	//float4x4 viewMatrix;
	//float4x4 projMatrix;
	float4x4 viewProjMatrix;
    //float4x4 clusterMatrix;
    //float4 screenParam;
    //float4 projectionParam;
};

cbuffer PerObject : register(b1)
{
    float4x4 objectToWorldMatrix;
}

float4 PositionObjectToWorld(float4 objectPos)
{
	return objectPos;// mul(objectPos, objectToWorldMatrix);
}

float4 PositionWorldToClip(float4 worldPos)
{
	return mul(viewProjMatrix, worldPos);
}

float3 DirectionObjectToWorld(float3 dir)
{
	return dir;
}

float4 PositionWorldToCluster(float4 worldPos)
{
    return worldPos;
}