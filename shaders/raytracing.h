#pragma once
#include "instanceconstants.h"
#include "random.h"
#include "ray.h"

//#define INVALID_INDEX 0xFFFFFFFF
#define INVALID_INDEX -1u
#define PI 3.1415926535897

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#define float2 glm::float2
#define min glm::min
#define max glm::max
#define inout
#endif

struct Vertex
{
    float3 position;
    uint32_t tangent;
    uint32_t normal;
    uint32_t uv;
    float pad0;
    float pad1;
};

struct BVHNode
{
    float3 minBounds; // for leaf x - primitive (triangle) id
    int instId; // instance id
    float3 maxBounds;
    int nodeOffset;
    int primitiveId;
    int pad0;
    int pad1;
    int pad2;
 };

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
bool RayTriangleIntersect(
    float3 orig,
    float3 dir,
    float3 v0,
    float3 e0,
    float3 e1,
    out float t,
    out float2 bCoord)
{
    t = 1e9;
    const float3 pvec = cross(dir.xyz, e1);

    float det = dot(e0, pvec);

    // Backface culling
    // if (det < 1e-6)
    // {
    //    return false;
    // }

    if (abs(det) < 1e-6)
    {
        return false;
    }

    float invDet = 1.0 / det;

    float3 tvec = orig - v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0)
    {
        return false;
    }

    float3 qvec = cross(tvec, e0);
    float v = dot(dir, qvec) * invDet;
    if (v < 0.0 || (u + v) > 1.0)
    {
        return false;
    }

    t = dot(e1, qvec) * invDet;

    if (t < 1e-6)
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

// BVHTriangle getTriangle(uint instanceId, uint primitiveId, in Accel accel)
// {
//     InstanceConstants instConst = accel.instanceConstants[(instanceId)];

//     uint i0 = accel.ib[(instConst.indexOffset + primitiveId * 3 + 0)];
//     uint i1 = accel.ib[(instConst.indexOffset + primitiveId * 3 + 1)];
//     uint i2 = accel.ib[(instConst.indexOffset + primitiveId * 3 + 2)];

//     // read and transform vertices, calculate edges
//     float3 v0 = mul(instConst.objectToWorld, float4(accel.vb[(i0)].position, 1.0)).xyz;
//     float3 e0 = mul(instConst.objectToWorld, float4(accel.vb[(i1)].position, 1.0)).xyz - v0;
//     float3 e1 = mul(instConst.objectToWorld, float4(accel.vb[(i2)].position, 1.0)).xyz - v0;

//     BVHTriangle res;
//     res.v0 = v0;
//     res.e0 = e0;
//     res.e1 = e1;

//     return res;
// }

BVHTriangle getTriangle(uint instanceId, uint primitiveId, in Accel accel)
{
    InstanceConstants instConst = accel.instanceConstants[NonUniformResourceIndex(instanceId)];

    uint i0 = accel.ib[NonUniformResourceIndex(instConst.indexOffset + primitiveId * 3 + 0)];
    uint i1 = accel.ib[NonUniformResourceIndex(instConst.indexOffset + primitiveId * 3 + 1)];
    uint i2 = accel.ib[NonUniformResourceIndex(instConst.indexOffset + primitiveId * 3 + 2)];

    // read and transform vertices, calculate edges
    float3 v0 = mul(instConst.objectToWorld, float4(accel.vb[NonUniformResourceIndex(i0)].position, 1.0)).xyz;
    float3 e0 = mul(instConst.objectToWorld, float4(accel.vb[NonUniformResourceIndex(i1)].position, 1.0)).xyz - v0;
    float3 e1 = mul(instConst.objectToWorld, float4(accel.vb[NonUniformResourceIndex(i2)].position, 1.0)).xyz - v0;

    BVHTriangle res;
    res.v0 = v0;
    res.e0 = e0;
    res.e1 = e1;

    return res;
}

// bool closestHit1(in Accel accel, in Ray ray, out Hit hit, int len)
// {
//     const float3 invdir = float3(1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z);
//     int dirIsNeg[3] = { invdir.x < 0, invdir.y < 0, invdir.z < 0 };

//     int toVisitOffset = 0, currentNodeIndex = 0;
//     int nodesToVisit[64];

//     uint32_t minHit = 1e9;
//     bool isFound = false;

//     uint32_t nodeIndex = 0;
//     while (nodeIndex != INVALID_INDEX && nodeIndex < len)
//     {
//         BVHNode node = accel.bvhNodes[NonUniformResourceIndex(nodeIndex)];
//         const uint32_t instanceIndex = node.instId;
//         float boxT = 1e9f;
//         //hit.t = 0.0;
//         if (instanceIndex != INVALID_INDEX) // leaf
//         {
//             const uint primitiveIndex = asuint(node.minBounds.x); // triangle index

//             BVHTriangle triangle = getTriangle(instanceIndex, primitiveIndex, accel);

//             float2 bary = float2(0.0);
//             float currT = 0.0;
//             bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, triangle.v0, triangle.e0, triangle.e1, currT, bary);
//             if (isIntersected && (currT < ray.o.w) && (currT < minHit))
//             {
//                 minHit = hit.t;
//                 hit.t = currT;
//                 hit.bary = bary;
//                 hit.instId = instanceIndex;
//                 hit.primId = primitiveIndex;
//                 isFound = true;
//             }
//         }
//         nodeIndex++;
//     }

//     return isFound;
// }

bool closestHit(in Accel accel, in Ray ray, out Hit hit)
{
    const float3 invdir = float3(1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z);

    float minHit = 1e9f;
    bool isFound = false;

    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = accel.bvhNodes[NonUniformResourceIndex(nodeIndex)];
        const uint32_t instanceIndex = node.instId;
        float boxT = 1e9f;
        //hit.t = 0.0;
        if (instanceIndex != INVALID_INDEX) // leaf
        {
            //const uint primitiveIndex = asuint(node.minBounds.x); // triangle index
            const uint primitiveIndex = node.primitiveId;

            BVHTriangle triangle = getTriangle(instanceIndex, primitiveIndex, accel);

            float2 bary = float2(0.0);
            float currT = 0.0;
            bool isIntersected = RayTriangleIntersect(ray.o.xyz, ray.d.xyz, triangle.v0, triangle.e0, triangle.e1, currT, bary);
            if (isIntersected && (currT < ray.o.w) && (currT < minHit))
            {
                minHit = hit.t;
                hit.t = currT;
                hit.bary = bary;
                hit.instId = instanceIndex;
                hit.primId = primitiveIndex;
                isFound = true;
            }
            nodeIndex = node.nodeOffset;
            continue;
        }
        else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds, boxT))
        {
            // if (boxT > ray.o.w) // check max ray trace distance: skip this node if collision far away
            // {
            //     nodeIndex = node.nodeOffset;
            //     continue;
            // }
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

#ifdef __cplusplus
#    undef float4
#    undef float3
#    undef float2
#    undef uint
#    undef max
#    undef min
#endif