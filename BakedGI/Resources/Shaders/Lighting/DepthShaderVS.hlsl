//felix wang
//Standard Shader for Physically based Shading by Cluster Lighting

#include "../Utils/Basics.hlsli"
#include "InputData.hlsli"

VertexOutput main( VertexInput v )
{
    VertexOutput o = (VertexOutput)0;
    o.posWS = PositionObjectToWorld(v.pos);
    o.normalWS = DirectionObjectToWorld(v.normal);
    o.tangentWS = DirectionObjectToWorld(v.tangent);
    o.binormalWS = cross(o.normalWS, o.tangentWS);
    o.posCluster = PositionWorldToCluster(o.posWS);
    o.uv = v.uv;
	return o;
}