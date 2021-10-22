#include "materials.h"
#include "pack.h"
#include "raytracing.h"
#include "reflectionparam.h"
#include "helper.h"

ConstantBuffer<ReflectionParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<int> gbInstId;
Texture2D<float4> gbUV;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;

Texture2D textures[BINDLESS_TEXTURE_COUNT];
SamplerState samplers[BINDLESS_SAMPLER_COUNT];

RWTexture2D<float4> output;

float3 calcReflection(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWPos[pixelIndex].xyz;

    float3 pointOnCamera = ubo.camPos;

    float3 V = normalize(pointOnCamera - wpos);
    float3 N = normalize(gbNormal[pixelIndex].xyz);

    Ray ray;
    ray.d = float4(normalize(reflect(-V, N)), 0.0);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    ray.o = float4(wpos + offset, 100);
    Hit hit;
    hit.t = 0.0;

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    int instId = gbInstId[pixelIndex];
    float2 matUV = gbUV[pixelIndex].xy;
    InstanceConstants constantsBase = instanceConstants[NonUniformResourceIndex(instId)];
    Material materialBase = materials[NonUniformResourceIndex(constantsBase.materialId)];
    float roughness = getRoughness(materialBase, matUV, textures, samplers);
    if (abs(roughness - 1.0) < 1e-5) {
        return (0, 0, 0);
    }

    if (closestHit(accel, ray, hit))
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

        float3 n = interpolateAttrib(n0, n1, n2, bcoords);

        float2 uv0 = unpackUV(accel.vb[i0].uv);
        float2 uv1 = unpackUV(accel.vb[i1].uv);
        float2 uv2 = unpackUV(accel.vb[i2].uv);

        float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

        float3 dcol = getBaseColor(material, uvCoord, textures, samplers);

        dcol = ((1 - roughness) * (2.5 * saturate(dot(n, ray.d.xyz)) + 0.25)) * dcol;

        return dcol;
    }

    return (0, 0, 0);
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex: SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = 0.f;

    color = calcReflection(pixelIndex);

    output[pixelIndex] = float4(color, 1.0);
}
