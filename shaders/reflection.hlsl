#include "lights.h"
#include "materials.h"
#include "pack.h"
#include "raytracing.h"
#include "reflectionparam.h"

ConstantBuffer<ReflectionParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> instId;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<RectLight> lights;

RWTexture2D<float4> output;

StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;

Texture2D textures[64]; // bindless
SamplerState samplers[15];

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

float getRoughness(Material material, float2 matUV)
{
    float roughness = material.roughnessFactor;
    if (material.texMetallicRoughness != -1)
    {
        roughness *= textures[NonUniformResourceIndex(material.texMetallicRoughness)].SampleLevel(samplers[NonUniformResourceIndex(material.sampMetallicRoughness)], matUV, 0).g;
    }
    return roughness;
}

float3 getBaseColor(Material material, float2 matUV)
{
    float3 dcol = material.baseColorFactor.rgb;
    if (material.texBaseColor != -1)
    {
        dcol *= textures[NonUniformResourceIndex(material.texBaseColor)].SampleLevel(samplers[NonUniformResourceIndex(material.sampBaseId)], matUV, 0).rgb;
    }
    return dcol;
}

float3 calcReflection(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWPos[pixelIndex].xyz;

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float2 rndUV = float2(rand(rngState), rand(rngState));

    RectLight curLight = lights[0];
    float3 pointOnCamera = ubo.camPos;

    float3 L = pointOnCamera + wpos;
    float3 N = normalize(gbNormal[pixelIndex].xyz);

    Ray ray;
    ray.d = float4(reflect(N, L), 0.0);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    float distToCamera = distance(pointOnCamera, wpos + offset);
    ray.o = float4(wpos + offset, distToCamera - 1e-5);
    Hit hit;
    hit.t = 0.0;

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;
    //float3 dcol = getBaseColor(material, uvCoord);

    if ((dot(N, L) > 0.0) && closestHit(accel, ray, hit))
    {
        float2 bcoords = hit.bary;
        InstanceConstants instConst = accel.instanceConstants[hit.instId];
        Material material = materials[instConst.materialId];

        uint i0 = accel.ib[instConst.indexOffset + hit.primId * 3 + 0];
        uint i1 = accel.ib[instConst.indexOffset + hit.primId * 3 + 1];
        uint i2 = accel.ib[instConst.indexOffset + hit.primId * 3 + 2];

        float3 n0 = unpackNormal(accel.vb[i0].normal);
        float3 n1 = unpackNormal(accel.vb[i1].normal);
        float3 n2 = unpackNormal(accel.vb[i2].normal);

        float3 n = n0 * (1 - bcoords.x - bcoords.y) + n1 * bcoords.x + n2 * bcoords.y;

        float2 uv0 = unpackUV(accel.vb[i0].uv);
        float2 uv1 = unpackUV(accel.vb[i1].uv);
        float2 uv2 = unpackUV(accel.vb[i2].uv);

        float2 uvCoord = uv0 * (1 - bcoords.x - bcoords.y) + uv1 * bcoords.x + uv2 * bcoords.y;

        float3 dcol = getBaseColor(material, uvCoord);
        float roughness = getRoughness(material, uvCoord);

        if (material.isLight == 1)
        {
            return dcol;
        }

        dcol = ((1 - roughness) * (2.5 * saturate(dot(n, ray.d.xyz)) + 0.25)) * dcol;

        return dcol;
    }

    return ( 0.f, 0.f, 0.f );
}

[numthreads(16, 16, 1)]
    [shader("compute")] void
    computeMain(uint2 pixelIndex
                : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = 0.f;

    color = calcReflection(pixelIndex);

    output[pixelIndex] = float4(color, 1.0);
}
