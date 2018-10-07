
#include "../Utils/Basics.hlsli"
#include "InputData.hlsli"

half4 main(VertexOutputDepth i) : SV_Target0
{
    return half4(i.clipPos.zzz, 1.0);
}