struct PositionData
{
    float3 posWorld;
    float3 viewDiretion;
    float3 normalWorld;
    float3 clusterId;
};

struct BRDFData
{
    half4 diffuse;
    half glossness;
    half metallic;
};

struct LocalLightData
{
	float4 color;
	float4 shadow;
};

struct LocalLightGeoData
{
	float4 pos;
	float4 dir;
};

struct DirectionalLightData
{
	float4 dir;
	float4 color;
	float4 shadow;
};

StructuredBuffer<LocalLightData> localLightsData;
StructuredBuffer<LocalLightGeoData> localLightsGeoData;

StructuredBuffer<DirectionalLightData> dirLightsData;
