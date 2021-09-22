#include "random.h"
#include "ray.h"
#include "lights.h"
#include "rtshadowparam.h"

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

ConstantBuffer<RtShadowParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<BVHTriangle> bvhTriangles;
StructuredBuffer<RectLight> lights;

RWTexture2D<float> output;

#define INVALID_INDEX 0xFFFFFFFF
#define PI 3.1415926535897

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
    float2 bary;
    float t;
    uint instId;
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
            const float4 v0 = bvhTriangles[NonUniformResourceIndex(primitiveIndex)].v0; // xyz - coord w - instId
            float2 bary;
            bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, v0.xyz, node.minBounds, node.maxBounds, hit.t, bary);
            if (isIntersected && (hit.t < ray.o.w)) // check max ray trace distance
            {
                hit.instId = asuint(v0.w);
                return true;
            }
        }
        else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds, boxT))
        {
            if (boxT > ray.o.w) // check max ray trace distance: skip this node if collision far away
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

bool closestHit(Ray ray, inout Hit hit)
{
    const float3 invdir = 1.0 / ray.d.xyz;
    
    uint32_t minHit = 1e9;
    bool isFound = false;

    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = bvhNodes[NonUniformResourceIndex(nodeIndex)];
        uint32_t primitiveIndex = node.instId;
        float boxT = 1e9f;
        if (primitiveIndex != INVALID_INDEX) // leaf
        {
            const float4 v0 = bvhTriangles[NonUniformResourceIndex(primitiveIndex)].v0;
            float2 bary;
            bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, v0.xyz, node.minBounds, node.maxBounds, hit.t, bary);
            if (isIntersected && (hit.t < ray.o.w) && (hit.t < minHit))
            {
                minHit = hit.t;
                hit.bary = bary;
                hit.instId = asuint(v0.w);
                isFound = true;
            }
        }
        else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds, boxT))
        {
            if (boxT > ray.o.w) // check max ray trace distance: skip this node if collision far away
            {
                nodeIndex = node.nodeOffset;
                continue;
            }
            ++nodeIndex;
            continue;
        }
        nodeIndex = node.nodeOffset;
    }

    return isFound;
}

float3 UniformSampleTriangle(float2 u) 
{
    float su0 = sqrt(u.x);
    float b0 = 1.0 - su0;
    float b1 = u.y * su0;
    return float3(b0, b1, 1.0 - b0 - b1);
}

float3 UniformSampleRect(RectLight l, float2 u)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return l.points[0].xyz + e1 * u.x + e2 * u.y;
}

float calcShadow(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 1.0; // no shadow
    float3 wpos = gbWPos[pixelIndex].xyz;

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float2 rndUV = float2(rand(rngState), rand(rngState));

    RectLight curLight = lights[0];
    float3 pointOnLight = UniformSampleRect(curLight, rndUV);

    float3 L = normalize(pointOnLight - wpos);
    float3 N = normalize(gbNormal[pixelIndex].xyz);
    
    Ray ray;
    ray.d = float4(L, 0.0);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    float distToLight = distance(pointOnLight, wpos + offset);
    ray.o = float4(wpos + offset, distToLight - 1e-5);
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
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    output[pixelIndex] = calcShadow(pixelIndex);
}
