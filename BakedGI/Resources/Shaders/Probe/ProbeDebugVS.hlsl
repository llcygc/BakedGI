
#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"

struct VertexInputProbe
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct VertexOutputProbe
{
    float4 clipPos : SV_Position;
};

[RootSignature(Probe_RootSig)]
VertexOutputProbe main(VertexInputProbe v)
{
    VertexOutputProbe o = (VertexOutputProbe) 0;
    float4 posWS = PositionObjectToWorld(float4(v.pos.xyz, 1.0));
    o.clipPos = PositionWorldToClip(posWS);
    return o;
}