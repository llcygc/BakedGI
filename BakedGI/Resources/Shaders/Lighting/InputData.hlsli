
struct VertexInput
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BITAGENT;
    float2 uv : TEXCOORD0;
};

struct VertexOutput
{
    float4 clipPos : SV_Position;
    float2 uv : TexCoord0;
    float4 posWS : TexCoord1;
    float3 normalWS : TexCoord2;
    float3 tangentWS : TexCoord3;
    float3 binormalWS : TexCoord4;
    float4 posCluster : TexCoord5;
};

struct VertexInputDepth
{
    float4 pos : POSITION;
};

struct VertexOutputDepth
{
    float4 clipPos : SV_Position;
};

struct VertexInputDepthCutout
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOutputDepthCutout
{
    float4 clipPos : SV_Position;
    float2 uv : TexCoord0;
};

struct VertexInputShadow
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
};

struct VertexOutputShadow
{
    float4 clipPos : SV_Position;
};

struct VertexInputShadowCutout
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VertexOutputShadowCutout
{
    float4 clipPos : SV_Position;
    float2 uv : TexCoord0;
};