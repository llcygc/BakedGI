
struct VertexInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BITANGENT;
};

struct VertexOutput
{
	float4 clipPos : SV_Position;
	float2 uv : TexCoord0;
	float4 posWS : TexCoord1;
	float3 normalWS : TexCoord2;
	float3 tangentWS : TexCoord3;
	float3 binormalWS : TexCoord4;
	float3 viewDir : TexCoord5;
	float4 posCluster : TexCoord6;
	//uint rtArraySlice : SV_RenderTargetArrayIndex;
};

struct PixelOutput
{
	float3 irradiance : SV_Target0;
	float2 normal : SV_Target1;
	float distance : SV_Target2;
};