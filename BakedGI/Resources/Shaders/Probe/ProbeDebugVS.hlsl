
#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"

struct VertexInputProbe
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexOutputProbe
{
    float4 clipPos : SV_Position;
    float3 cubeUV : TexCoord0;
};

[RootSignature(Probe_RootSig)]
VertexOutputProbe main(VertexInputProbe v)
{
    VertexOutputProbe o = (VertexOutputProbe) 0;
    float4 posWS = PositionObjectToWorld(float4(v.pos.xyz, 1.0));
    o.clipPos = PositionWorldToClip(posWS);
    o.cubeUV = v.pos.xyz;
    return o;
}