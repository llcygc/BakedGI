#include "ProbeRS.hlsli"
#include "TraceModule.hlsli"

[RootSignature(ProbeTrace_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

}