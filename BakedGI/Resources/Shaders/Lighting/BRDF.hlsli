
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
    half roughness;
    half metallic;
};

float DisneyDiffuse()
{
}

float GGXSpecular()
{
}