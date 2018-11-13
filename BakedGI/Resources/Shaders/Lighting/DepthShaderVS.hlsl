//felix wang
//Standard Shader for Physically based Shading by Cluster Lighting

#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"
#include "InputData.hlsli"

[RootSignature(Standard_RootSig)]
VertexOutputDepth main(VertexInputDepth v)
{
    VertexOutputDepth o = (VertexOutputDepth) 0;
    float4 posWS = PositionObjectToWorld(float4(v.pos.xyz, 1.0));
    o.clipPos = PositionWorldToClip(posWS);
	return o;
}