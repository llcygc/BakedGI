#include "InputData.hlsli"
#include "ProbeRS.hlsli"
#include "../Utils/Basics.hlsli"

cbuffer ProbeParams : register(b1)
{
	uint sliceIndex;
	uint unused0;
	uint unused1;
	uint unused2;
};

[RootSignature(ProbeRender_RootSig)]
VertexOutput main(VertexInput v)
{
	VertexOutput o = (VertexOutput)0;
	o.posWS = PositionObjectToWorld(float4(v.pos, 1.0));
	o.clipPos = PositionWorldToClip(o.posWS);
	o.normalWS = DirectionObjectToWorld(v.normal);
	o.tangentWS = DirectionObjectToWorld(v.tangent);
	o.binormalWS = DirectionObjectToWorld(v.binormal);
    o.posCluster = float4(cameraPos, sliceIndex); //PositionWorldToCluster(o.posWS);
	o.viewDir = cameraPos - o.posWS.xyz;
	o.uv = v.uv;
	//o.rtArraySlice = probeParams.sliceIndex;
	return o;
}