//felix wang
//Standard Shader for Physically based Shading by Cluster Lighting

#include "../Utils/Basics.hlsli"
#include "InputData.hlsli"

VertexOutput main( VertexInput v )
{
    VertexOutput o = (VertexOutput)0;
    o.posWS = PositionObjectToWorld(float4(v.pos, 1.0));
    o.normalWS = DirectionObjectToWorld(v.normal);
    o.tangentWS = DirectionObjectToWorld(v.tangent);
    o.binormalWS = cross(o.normalWS, o.tangentWS);
    o.posCluster = PositionWorldToCluster(o.posWS);
    o.uv = v.uv;
	return o;
}