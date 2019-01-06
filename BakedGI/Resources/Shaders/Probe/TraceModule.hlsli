
#include "../Utils/Octahedral.hlsli"

#define SAMPLE_COUNT 64
#define SAMPLE_DIM 8

#define TRACE_HIT 0
#define TRACE_MISS 1
#define TRACE_UNKNONW 2

#define RED half3(1, 0, 0)
#define GREEN half3(0, 1, 0)
#define BLUE half3(0, 0, 1)
#define PURPLE half3(1, 0, 1)
#define YELLOW half3(1, 1, 0)
#define ORANGE half3(1, 0.5, 0)
#define WHITE half3(1, 1, 1)
#define BLACK half3(0, 0, 0)

const float minThickness = 0.03; // meters
const float maxThickness = 0.50; // meters

#define rayBumpEpsilon 0.005f // meters
#define PI 3.14159

struct ProbeData
{
    float3 probePos;
};

cbuffer ProbeParams : register(b1)
{
    float4 probeRes;
    float4 screenParams;
    float4 probeParams;
    float4 probePosMin;
    float4 probePosMax;
    float4 probeZParam;
};

Texture2DArray<float3> irradianceOctanMap : register(t0);
Texture2DArray<float2> normalOctanMap : register(t1);
Texture2DArray<float> distanceOctanMap : register(t2);
Texture2DArray<float> distanceMinMipOctaMap : register(t3);

StructuredBuffer<ProbeData> probeDatas : register(t4);

Texture2D<float4> GBuffer0 : register(t32);
Texture2D<float4> GBuffer1 : register(t33);
Texture2D<float4> GBuffer2 : register(t34);
Texture2D<float> depthBuffer : register(t35);

RWTexture2D<float3> traceResult;

SamplerState sampler0 : register(s0);

int CoordToIndex(int3 coord)
{
    return coord.x * probeParams.y * probeParams.z + coord.y * probeParams.z + coord.z;
}

half3 IndexToCoord(int index)
{
    half3 coord = 0;
    coord.z = index % ((int) probeParams.z);
    coord.y = floor(index % ((int) probeParams.z * (int) probeParams.y) / probeParams.z);
    coord.x = floor(index / ((int) probeParams.z * (int) probeParams.y));

    return coord / probeParams;
}

void FetchProbeIndices(float3 worldPos, out int4 indices1, out int4 indices2)
{
    float3 totalDist = ProbeMax.xyz - ProbeMin.xyz;
    float3 currentDist = worldPos - ProbeMin.xyz;
    float3 coord = saturate(currentDist / totalDist);

    coord = floor(coord * (probeParams - 1));

    if (coord.x == probeParams.x - 1)
        coord.x -= 1;
    if (coord.y == probeParams.y - 1)
        coord.y -= 1;
    if (coord.z == probeParams.z - 1)
        coord.z -= 1;

    indices1.x = CoordToIndex(coord);
    indices1.y = CoordToIndex(coord + int3(1, 0, 0));
    indices1.z = CoordToIndex(coord + int3(0, 1, 0));
    indices1.w = CoordToIndex(coord + int3(1, 1, 0));

    indices2.x = CoordToIndex(coord + int3(0, 0, 1));
    indices2.y = CoordToIndex(coord + int3(1, 0, 1));
    indices2.z = CoordToIndex(coord + int3(0, 1, 1));
    indices2.w = CoordToIndex(coord + int3(1, 1, 1));
}

half3 FetchProbeIndices(float3 worldPos)
{
    float3 totalDist = ProbeMax.xyz - ProbeMin.xyz;
    float3 currentDist = worldPos - ProbeMin.xyz;
    float3 coord = saturate(currentDist / totalDist);

    coord = floor(coord * (probeParams.xyz - 1));
    if (coord.x == probeParams.x - 1)
        coord.x -= 1;
    if (coord.y == probeParams.y - 1)
        coord.y -= 1;
    if (coord.z == probeParams.z - 1)
        coord.z -= 1;
	
    return coord / probeParams;
}


uint FetchClosestProbe(float3 worldPos, float3 dir, uint indices[8])
{
    float cosTheta = 0;
    uint index = 0;
    for (uint i = 0; i < 8; i++)
    {
        ProbeData tempData = probeDatas[indices[i]];
        float3 probePos = tempData.probePos;
        float3 localPos = probePos - worldPos;
        float3 probeDir = normalize(localPos);
        float tempCos = dot(probeDir, dir);
        if (tempCos > cosTheta)
        {
            index = indices[i];
            cosTheta = tempCos;
        }
    }

    return index;
}

float CubeMapDist(float3 localPos)
{
    float dist;
    float3 dir = normalize(localPos);

    if (abs(dir.z) >= abs(dir.x) && abs(dir.z) >= abs(dir.y))
    {
        dist = abs(localPos.z);
    }
    else if (abs(dir.y) >= abs(dir.x))
    {
        dist = abs(localPos.y);
    }
    else
    {
        dist = abs(localPos.x);
    }

    return dist;
}

float3 DebugCloset(float3 worldPos, float3 dir, uint indices[8])
{
    float cosTheta = 0;
    uint index = 0;
    uint i = 0;
    ProbeData tempData = probeDatas[indices[i]];
    float3 probePos = tempData.probePos;
    float3 localPos = probePos - worldPos;
    float3 probeDir = normalize(localPos);
    float tempCos = abs(dot(probeDir, dir));
    return tempCos;
}

/** Two-element sort: maybe swaps a and b so that a' = min(a, b), b' = max(a, b). */
void minSwap(inout float a, inout float b)
{
    float temp = min(a, b);
    b = max(a, b);
    a = temp;
}


/** Sort the three values in v from least to
greatest using an exchange network (i.e., no branches) */
void sort(inout float3 v)
{
    minSwap(v[0], v[1]);
    minSwap(v[1], v[2]);
    minSwap(v[0], v[1]);
}

void SortProbes(float3 worldPos, float3 dir, inout uint indices[8])
{

}

inline float sqrLength(float3 vec)
{
    return dot(vec, vec);
}


inline float sqrLength(float2 vec)
{
    return dot(vec, vec);
}

/** Returns the distance along v from the origin to the intersection
with ray R (which it is assumed to intersect) */
float distanceToIntersection(in float3 origin, in float3 dir, in float3 v)
{
    float numer;
    float denom = v.y * dir.z - v.z * dir.y;

    if (abs(denom) > 0.1)
    {
        numer = origin.y * dir.z - origin.z * dir.y;
    }
    else
    {
		// We're in the yz plane; use another one
        numer = origin.x * dir.y - origin.y * dir.x;
        denom = v.x * dir.y - v.y * dir.x;
    }

    return numer / denom;
}

float distanceToIntersectionFix(in float3 origin, in float3 dir, in float3 v)
{
    float3 e = v;
    float3 f = dir;
    float3 g = origin;

    float3 H = cross(f, g);
    float3 K = cross(f, e);

    float h = length(H);
    float k = length(K);

    float result = h / k;

    if (dot(H, K) <= 0)
        result *= -1;

    return max(0, result);
	
}

bool LowResTrace(uint index, float3 localPos, float3 dir, inout float2 startUV, in float2 segEndUV, inout float2 hitEndUV, inout half3 debugColor)
{
    hitEndUV = startUV;
    float2 startCoord = startUV * probeRes.zz;
    float2 segEndCoord = segEndUV * probeRes.zz;

    float texelSize = 1.0f / probeRes.z;
    float2 delta = segEndCoord - startCoord;

    segEndCoord += ((sqrLength(startCoord - segEndCoord) < 0.0001) ? 0.01 : 0.0);

    float steps = max(abs(delta.x), abs(delta.y));
    float dist = length(delta);
    float2 traceDir = normalize(delta); // / dist;

    float traceDist = 0;
    float3 dirBefore = octDecode(startUV * 2.0 - 1.0);
    dirBefore.y *= -1;
    float distBefore = max(0.0, distanceToIntersectionFix(localPos, dir, dirBefore));
    float2 currentCoord = startCoord;
    int traceStepCount = 0;
    while (traceDist < dist)
    {
        traceStepCount++;
        float sceneMinDist = distanceMinMipOctaMap.Load(int4(currentCoord, index, 0)); //LOAD_TEXTURE2D_ARRAY(DistMapMinMipOctan, floor(currentCoord), index) * ProbeProjectonParam.y;
        
        float2 deltaToPixelEdge = frac(currentCoord) - (sign(delta) * 0.5 + 0.5);
        float2 distToPixelEdge = deltaToPixelEdge * (-1.0f / traceDir);
        float traceStep = min(abs(distToPixelEdge.x), abs(distToPixelEdge.y));
        
        float2 hitEndCoord = (currentCoord + traceDir * traceStep);
        float2 hitEndUV = hitEndCoord * probeRes.ww;
        if (length(hitEndUV * probeRes.zz - startCoord) > dist)
            hitEndUV = segEndUV;

        float3 dirEnd = octDecode(hitEndUV * 2.0 - 1.0);
        dirEnd.y *= -1;
        float distAfter = max(0.0, distanceToIntersectionFix(localPos, dir, dirEnd));
        
        float2 detlaCoord = abs(hitEndCoord - currentCoord);
        debugColor = half3(hitEndUV, 0.0f); //sceneMinDist / ProbeProjectonParam.y;
        if (min(distBefore, distAfter) > sceneMinDist)
        {
            startUV = currentCoord * probeRes.ww;
            debugColor = half3(startUV, 0); //sceneMinDist / ProbeProjectonParam.y;
            return true;
        }

        const float epsilon = 0.001; // pixels
        currentCoord += traceDir * (traceStep + 0.001f);
        traceDist += (traceStep + 0.001f);
    }

    startUV = segEndUV;
    return false;
}

uint HighResTrace(uint index, float3 localPos, float3 dir, inout float tMin, inout float tMax, in float2 startUV, in float2 hitEndUV, out float2 hitUV, out float3 hitNormal, inout half3 debugColor)
{
    
    float2 startCoord = startUV * probeRes.xx;
    float2 endCoord = hitEndUV * probeRes.xx;

    endCoord += sqrLength(startUV - hitEndUV) > 0.0001 ? 0.01 : 0.0;
    float2 delta = endCoord - startCoord;
    float2 traceDir = normalize(delta);
    float dist = length(delta);

    float traceStep = dist / max(abs(delta.x), abs(delta.y));
            
    float3 dirBefore = octDecode((startUV) * 2.0 - 1.0);
    dirBefore.y *= -1;
    float distBefore = max(0.0, distanceToIntersectionFix(localPos, dir, dirBefore));
    
    debugColor = PURPLE;
    float traceDist = 0;
    while (traceDist < dist)
    {
        float2 currentUV = saturate(startUV + traceDir * min(traceDist + traceStep * 0.5, dist));
        if (all(currentUV >= 0) && all(currentUV <= 1))
        {
            int2 currentCoord = (int2) (currentUV * probeRes.xx);
            float sceneDist = SAMPLE_TEXTURE2D_ARRAY_LOD(DistMapOctan, sampler_DistMapOctan, currentUV, index, 0) * ProbeProjectonParam.y;

            float2 afterUV = startUV + traceDir * min(traceDist + traceStep, dist);
            float3 dirAfter = octDecode(currentUV * 2.0 - 1.0);
            dirAfter.y *= -1;
            float distAfter = max(0.0, distanceToIntersectionFix(localPos, dir, dirAfter));

            float maxRayDist = max(distBefore, distAfter);

            if (maxRayDist >= sceneDist)
            {
                float minRayDist = min(distBefore, distAfter);

                float distanceFromProbeToRay = (minRayDist + maxRayDist) * 0.5;
                float3 directionFromProbe = octDecode(currentUV * 2.0 - 1.0);
                directionFromProbe.y *= -1;

                float3 probeSpaceHitPoint = sceneDist * directionFromProbe;
                float distAlongRay = dot(probeSpaceHitPoint - localPos, dir);

                float2 normalOcta = LOAD_TEXTURE2D_ARRAY(NormalMapOctan, currentCoord, index).xy;
                float3 normal = octDecode(normalOcta * 2.0 - 1.0);
						// Only extrude towards and away from the view ray, not perpendicular to it
						// Don't allow extrusion TOWARDS the viewer, only away
                float surfaceThickness = minThickness
							+ (maxThickness - minThickness) *

							// Alignment of probe and view ray
							max(dot(dir, directionFromProbe), 0.0) *

							// Alignment of probe and normal (glancing surfaces are assumed to be thicker because they extend into the pixel)
							(2 - abs(dot(dir, normal))) *

							// Scale with distance along the ray
							clamp(distAlongRay * 0.1, 0.05, 1.0);

                if ((minRayDist < sceneDist + surfaceThickness) && (dot(normal, dir) < 0))
                {
                    debugColor = BLUE; // any(normal < half3(0, 0, 0)) ? BLUE : BLACK;// dot(normal, dir); //half3(currentUV, 0);
							// Two-sided hit
							// Use the probe's measure of the point instead of the ray distance, since
							// the probe is more accurate (floating point precision vs. ray march iteration/oct resolution)
                    tMax = distAlongRay;
                    hitUV = currentUV;
                    hitNormal = normal;
                    return TRACE_HIT;
                }
                else
                {
                    debugColor = RED;
                    hitUV = 0;
							// "Unknown" case. The ray passed completely behind a surface. This should trigger moving to another
							// probe and is distinguished from "I successfully traced to infinity"

							// Back up conservatively so that we don't set tMin too large
                    float3 probeSpaceHitPointBefore = distBefore * dirBefore;
                    float distAlongRayBefore = dot(probeSpaceHitPointBefore - localPos, dir);

							// Max in order to disallow backing up along the ray (say if beginning of this texel is before tMin from probe switch)
							// distAlongRayBefore in order to prevent overstepping
							// min because sometimes distAlongRayBefore > distAlongRay
                    tMin = max(tMin, min(distAlongRay, distAlongRayBefore));

                    return TRACE_UNKNONW;
                }
            }
            

            distBefore = distAfter;
            traceDist += traceStep;
        }
        else
        {
            hitUV = 0;
            break;
        }
    }

    return TRACE_MISS;
}

//TRACE_HIT 0
//TRACE_MISS 1
//TRACE_UNKNONW 2
uint TraceSingleProbe(uint index, float3 worldPos, float3 dir, inout float tMin, inout float tMax, out float2 hitUV, out float3 hitNormal, out half3 debugColor)
{
    ProbeData pData = probeDatas[index];
    float3 localPos = worldPos - pData.probePos;
    debugColor = 0;

    float boundaryTs[5];

    boundaryTs[0] = 0.0f;

    float3 t = localPos * -(1.0f / dir);
    sort(t);

    for (int i = 0; i < 3; ++i)
    {
        boundaryTs[i + 1] = clamp(t[i], 0.0f, 1000.0f);
    }

    boundaryTs[4] = 1000.0f;

    const float degenerateEpsilon = 0.001f;

    [loop]
    for (int i = 0; i < 4; i++)
    {
        float t0 = boundaryTs[i];
        float t1 = boundaryTs[i + 1];
        [branch]
        if (abs(t0 - t1) >= degenerateEpsilon)
        {
            float3 startPoint = localPos + dir * (t0 + rayBumpEpsilon);
            float3 endPoint = localPos + dir * (t1 - rayBumpEpsilon);

            if (sqrLength(startPoint) < 0.001)
                startPoint = dir;

            startPoint.y *= -1;
            endPoint.y *= -1;

            float2 startUV = octEncode(normalize(startPoint)) * 0.5 + 0.5;
            float2 endUV = octEncode(normalize(endPoint)) * 0.5 + 0.5;
            float2 hitEndUV;

            [branch]
            if (LowResTrace(index, localPos, dir, startUV, endUV, hitEndUV, debugColor))
            {
                uint result = HighResTrace(index, localPos, dir, tMin, tMax, startUV, hitEndUV, hitUV, hitNormal, debugColor);
                if (result != TRACE_MISS)
                    return result;
            }
        }
    }

    return TRACE_MISS;
}

half3 GlobalIllumination_Trace(BRDFData_Direct brdfData, half3 normalWS, half3 tangent, half3 binormal, half3 viewDirectionWS, float3 worldPos)
{
    uint4 indices1 = 0;
    uint4 indices2 = 0;

    FetchProbeIndices(worldPos, indices1, indices2);
    uint indices[8] = { indices1.x, indices1.y, indices1.z, indices1.w, indices2.x, indices2.y, indices2.z, indices2.w };

    half3 color = 0;

    int count = 0;
    float step = 0.5f;
    [loop]
    for (int i = 0; i < 1 /*SAMPLE_COUNT*/; i++)
    {
        float divX = floor(i / SAMPLE_DIM) + 1;
        float divY = i % SAMPLE_DIM + 1;
        float theta = 2 * PI * (divX / (SAMPLE_DIM + 1));
        float phi = 0.25 * PI * (divY / (SAMPLE_DIM + 1));

        float x = sin(phi) * cos(theta);
        float y = sin(phi) * sin(theta);
        float z = cos(phi);

        float3 dir = normalWS /** z + binormal * y + tangent * x*/;
        half3 skyColor;

        worldPos += dir * rayBumpEpsilon;

        SortProbes(worldPos, dir, indices);
        
        //TRACE_HIT 0
        //TRACE_MISS 1
        //TRACE_UNKNONW 2
        uint result = TRACE_UNKNONW;
        bool traceComplete = false;
        half3 radColor = 0;

        float tMin = 0.0f;
        float tMax = 1000.0f;
        float2 hitUV = 0;
        float3 normal = 0;
        half3 debugColor = 0;

        uint hitIndex;
        
        for (uint j = 0; j < 8; j++)
        {
            hitIndex = indices[j];
            result = TraceSingleProbe(indices[j], worldPos, dir, tMin, tMax, hitUV, normal, debugColor);

            if (result != TRACE_UNKNONW)
                break;
        }

        if (result == TRACE_HIT)
        {
            half3 hitColor = LOAD_TEXTURE2D_ARRAY(RadMapOctan, hitUV * probeRes.xx, hitIndex);

			//color += saturate(dot(dir, normalWS)) /** saturate(dot(-dir, hitNormal))*/ * hitColor / (2 * PI * max(1.0f, tMax * tMax));
        }

        if (result == TRACE_MISS)
            color = PURPLE;
        color = debugColor;
    }
	
    return (color) /* * brdfData.diffuse*/;
}