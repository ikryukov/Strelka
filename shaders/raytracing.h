#pragma once
#include "random.h"
#include "ray.h"

#define INVALID_INDEX 0xFFFFFFFF
#define PI 3.1415926535897

struct Vertex
{
    float3 position;
    uint32_t tangent;
    uint32_t normal;
    uint32_t uv;
    float pad0;
    float pad1;
};

struct InstanceConstants
{
    float4x4 model;
    float4x4 normalMatrix;
    int32_t materialId;
    int32_t indexOffset;
    int32_t indexCount;
    int32_t pad2;
};

struct BVHNode
{
    float3 minBounds; // for leaf x - primitive (triangle) id
    int instId;
    float3 maxBounds;
    int nodeOffset;
};

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
    uint primId;
};

struct BVHTriangle
{
    float3 v0;
    float3 e0;
    float3 e1;
};

struct Accel
{
    StructuredBuffer<BVHNode> bvhNodes;
    StructuredBuffer<InstanceConstants> instanceConstants;
    StructuredBuffer<Vertex> vb;
    StructuredBuffer<uint> ib;
};

BVHTriangle getTriangle(uint instanceId, uint primitiveId, in Accel accel)
{
    InstanceConstants instConst = accel.instanceConstants[instanceId];

    uint i0 = accel.ib[instConst.indexOffset + primitiveId * 3 + 0];
    uint i1 = accel.ib[instConst.indexOffset + primitiveId * 3 + 1];
    uint i2 = accel.ib[instConst.indexOffset + primitiveId * 3 + 2];

    // read and transform vertices, calculate edges
    float3 v0 = mul(instConst.model, float4(accel.vb[i0].position, 1.0)).xyz;
    float3 e0 = mul(instConst.model, float4(accel.vb[i1].position, 1.0)).xyz - v0;
    float3 e1 = mul(instConst.model, float4(accel.vb[i2].position, 1.0)).xyz - v0;

    BVHTriangle res;
    res.v0 = v0;
    res.e0 = e0;
    res.e1 = e1;

    return res;
}

bool closestHit(in Accel accel, in Ray ray, inout Hit hit)
{
    const float3 invdir = 1.0 / ray.d.xyz;

    uint32_t minHit = 1e9;
    bool isFound = false;

    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = accel.bvhNodes[NonUniformResourceIndex(nodeIndex)];
        const uint32_t instanceIndex = node.instId;
        float boxT = 1e9f;
        if (instanceIndex != INVALID_INDEX) // leaf
        {
            const uint32_t primitiveIndex = asuint(node.minBounds.x); // triangle index

            BVHTriangle triangle = getTriangle(instanceIndex, primitiveIndex, accel);

            float2 bary;
            bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, triangle.v0, triangle.e0, triangle.e1, hit.t, bary);
            if (isIntersected && (hit.t < ray.o.w) && (hit.t < minHit))
            {
                minHit = hit.t;
                hit.bary = bary;
                hit.instId = instanceIndex;
                hit.primId = primitiveIndex;
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

bool anyHit(in Accel accel, in Ray ray, inout Hit hit)
{
    const float3 invdir = 1.0 / ray.d.xyz;
    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = accel.bvhNodes[NonUniformResourceIndex(nodeIndex)];
        uint32_t instanceIndex = node.instId;
        float boxT = 1e9f;
        if (instanceIndex != INVALID_INDEX) // leaf
        {
            const uint32_t primitiveIndex = asuint(node.minBounds.x); // triangle index

            BVHTriangle triangle = getTriangle(instanceIndex, primitiveIndex, accel);

            float2 bary;
            bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, triangle.v0, triangle.e0, triangle.e1, hit.t, bary);
            if (isIntersected && (hit.t < ray.o.w)) // check max ray trace distance
            {
                hit.instId = instanceIndex;
                hit.primId = primitiveIndex;
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
