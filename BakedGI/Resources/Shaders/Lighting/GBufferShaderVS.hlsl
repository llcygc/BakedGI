//felix wang
//Standard Shader for Physically based Shading by Cluster Lighting

#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"
#include "InputData.hlsli"

[RootSignature(GBuffer_RootSig)]
VertexOutput main(VertexInput v)
{
	VertexOutput o = (VertexOutput)0;
	o.posWS = PositionObjectToWorld(float4(v.pos, 1.0));
	o.clipPos = PositionWorldToClip(o.posWS);
	o.normalWS = DirectionObjectToWorld(v.normal);
	o.tangentWS = DirectionObjectToWorld(v.tangent);
	o.binormalWS = DirectionObjectToWorld(v.binormal);
	o.posCluster = PositionWorldToCluster(o.posWS);
	o.viewDir = cameraPos - o.posWS.xyz;
	o.uv = v.uv;
	return o;
}