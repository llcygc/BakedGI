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

Buffer<LocalLightData> localLightsData;
Buffer<LocalLightGeoData> localLightsGeoData;

Buffer<DirectionalLightData> dirLightsData;