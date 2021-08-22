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
    float4 o; // xyz - origin, w - max trace distance
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

bool intersectRayBox(Ray r, float3 invdir, float3 pmin, float3 pmax, inout float t)
{
    const float3 f = (pmax.xyz - r.o.xyz) * invdir;
    const float3 n = (pmin.xyz - r.o.xyz) * invdir;

    const float3 tmax = max(f, n);
    const float3 tmin = min(f, n);

    const float t1 = min(tmax.x, min(tmax.y, tmax.z));
    const float t0 = max(max(tmin.x, max(tmin.y, tmin.z)), 0.0f);

    t = t0;
    return t1 >= t0;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
bool RayTriangleIntersect(
    const float3 orig,
    const float3 dir,
    const float3 v0,
    const float3 e0,
    const float3 e1,
    inout float t,
    inout float2 bCoord)
{
    const float3 pvec = cross(dir.xyz, e1);

    float det = dot(e0, pvec);

    // Backface culling
    if (det < 1e-6)
    {
        return false;
    }

    // if (abs(det) < 1e-6)
    // {
    //     return false;
    // }

    float invDet = 1.0 / det;

    float3 tvec = orig - v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0)
    {
        return false;
    }

    float3 qvec = cross(tvec, e0);
    float v = dot(dir, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0)
    {
        return false;
    }

    t = dot(e1, qvec) * invDet;

    if (t < 0.0)
    {
        return false;
    }

    bCoord.x = u;
    bCoord.y = v;

    return true;
}

struct Hit
{
    float t;    
};

bool anyHit(Ray ray, inout Hit hit)
{
    const float3 invdir = 1.0 / ray.d.xyz;
    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = bvhNodes[NonUniformResourceIndex(nodeIndex)];
        uint32_t primitiveIndex = node.instId;
        float boxT = 1e9f;
        if (primitiveIndex != INVALID_INDEX) // leaf
        {
            const float3 v0 = bvhTriangles[NonUniformResourceIndex(primitiveIndex)].v0.xyz;
            float2 bary;
            bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, v0, node.minBounds, node.maxBounds, hit.t, bary);
            if (isIntersected && (hit.t < ray.o.w))
            {
                return true;
            }
        }
        else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds, boxT))
        {
            if (boxT > ray.o.w)
            {
                nodeIndex = node.nodeOffset;
                continue;
            }
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
    //float3 bary = float3(rndUV.x, rndUV.y, 1.0 - rndUV.x - rndUV.y);

    float3 pointOnLight = bary.z * lights[0].v0 + bary.x * lights[0].v1 + bary.y * lights[0].v2;
    //float3 pointOnLight = float3(0.85, 0.65, 1.01);
    //float3 pointOnLight = float3(6.0, 20.0, 6.0);

    float3 L = normalize(pointOnLight - wpos);
    float3 N = normalize(gbNormal[pixelIndex].xyz);
    //float3 N = float3(0.0, 0.0, 1.0);
    
    Ray ray;
    ray.d = float4(L, 0.0);
    //ray.d = float4(N, 0.0);
    float3 offset = N * 1e-5;
    float distToLight = distance(pointOnLight, wpos + offset);
    ray.o = float4(wpos + offset, distToLight);
    Hit hit;
    hit.t = 0.0;
    if ((dot(N, L) > 0.0) && anyHit(ray, hit))
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
