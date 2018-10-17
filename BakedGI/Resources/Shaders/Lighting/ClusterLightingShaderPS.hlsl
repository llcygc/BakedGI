
#include "../Utils/Basics.hlsli"
#include "../Utils/Resouces.hlsli"
#include "InputData.hlsli"

[RootSignature(Standard_RootSig)]
float3 main(VertexOutput i) : SV_Target0
{
    return float3(1, 0, 0);
}