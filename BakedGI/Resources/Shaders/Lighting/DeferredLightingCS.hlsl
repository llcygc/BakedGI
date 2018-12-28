#include "DeferredLightingRS.hlsli"

Texture2D<half4> GBuffer0 : register(t0);
Texture2D<half4> GBuffer1 : register(t1);
Texture2D<half4> GBuffer2 : register(t2);
Texture2D<float> DepthBuffer : register(t3);
RWTexture2D<float3> OutColor : register(u0);

[RootSignature(Deferred_RootSig)]
[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 diffuse = GBuffer0[DTid.xy];
    float4 normal = GBuffer1[DTid.xy];
    float4 mso = GBuffer2[DTid.xy];
    float depth = DepthBuffer[DTid.xy];

    OutColor[DTid.xy] =  GBuffer0[DTid.xy].rgb;
}