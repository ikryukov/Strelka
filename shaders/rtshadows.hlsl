#include "random.h"

cbuffer ubo
{
    float4x4 viewToProj;
    float4x4 worldToView;
    float3 CameraPos;
    uint frameNumber;
    uint2 dimension;
    float pad0;
    float pad1;
}

struct BVHNode 
{
    float3 minBounds;
    int instId;
    float3 maxBounds;
    int nodeOffset;
};

struct BVHTriangle
{
    float4 v0;
};

struct Light
{
    float3 v0;
    float pad0;
    float3 v1;
    float pad1;
    float3 v2;
    float pad2;
};

struct Ray
{
    float4 o;
    float4 d;
};

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<BVHTriangle> bvhTriangles;
StructuredBuffer<Light> lights;

RWTexture2D<float> output;

#define INVALID_INDEX 0xFFFFFFFF
#define PI 3.1415926535897

bool intersectRayBox(Ray r, float3 invdir, float3 pmin, float3 pmax)
{
    const float3 f = (pmax.xyz - r.o.xyz) * invdir;
    const float3 n = (pmin.xyz - r.o.xyz) * invdir;

    const float3 tmax = max(f, n);
    const float3 tmin = min(f, n);

    const float t1 = min(tmax.x, min(tmax.y, tmax.z));
    const float t0 = max(max(tmin.x, max(tmin.y, tmin.z)), 0.0f);

    return t1 >= t0;
}

bool intersectRayTri(Ray r, float3 v0, float3 e0, float3 e1)
{
    const float3 s1 = cross(r.d.xyz, e1);
    const float  invd = 1.0 / (dot(s1, e0));
    const float3 d = r.o.xyz - v0;
    const float  b1 = dot(d, s1) * invd;
    const float3 s2 = cross(d, e0);
    const float  b2 = dot(r.d.xyz, s2) * invd;
    const float temp = dot(e1, s2) * invd;

    if (b1 < 0.0 || b1 > 1.0 || b2 < 0.0 || b1 + b2 > 1.0 || temp < 0.0 || temp > r.o.w)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool anyHit(Ray ray)
{
    const float3 invdir = 1.0 / ray.d.xyz;
    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = bvhNodes[NonUniformResourceIndex(nodeIndex)];
        uint32_t primitiveIndex = node.instId;
        if (primitiveIndex != INVALID_INDEX) // leaf
        {
            const float3 v0 = bvhTriangles[NonUniformResourceIndex(primitiveIndex)].v0.xyz;
            if (intersectRayTri(ray, v0, node.minBounds, node.maxBounds))
            {
                return true;
            }
        }
        else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds))
        {
            ++nodeIndex;
            continue;
        }
        nodeIndex = node.nodeOffset;
    }
    return false;
}

float3 UniformSampleTriangle(float2 u) 
{
    float su0 = sqrt(u.x);
    float b0 = 1.0 - su0;
    float b1 = u.y * su0;
    return float3(b0, b1, 1.0 - b0 - b1);
}

float calcShadow(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWPos[pixelIndex].xyz;


    uint rngState = initRNG(pixelIndex, dimension, frameNumber);

    float2 rndUV = float2(rand(rngState), rand(rngState));
    float3 bary = UniformSampleTriangle(rndUV);

    float3 pointOnLight = (1.0 - bary.x - bary.y) * lights[0].v0 + bary.x * lights[0].v1 
    + bary.y * lights[0].v2;

    float3 L = normalize(pointOnLight - wpos);
    float3 N = gbNormal[pixelIndex].xyz;
    
    Ray ray;

    ray.d = float4(L, 0.0);
    float3 offset = N * 1e-5;
    ray.o = float4(wpos + offset, 1e9);

    if (dot(N, L) > 0.0 && anyHit(ray))
    {
        return 0.1;
    }
    return 1.0;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= dimension.x || pixelIndex.y >= dimension.y)
    {
        return;
    }
    output[pixelIndex] = calcShadow(pixelIndex);
}
