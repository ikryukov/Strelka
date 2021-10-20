#include "lights.h"
#include "rtshadowparam.h"
#include "raytracing.h"

ConstantBuffer<RtShadowParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<RectLight> lights;

StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;

RWTexture2D<float> output;

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

    float color = 0.0;

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float2 rndUV = float2(rand(rngState), rand(rngState));

    RectLight curLight = lights[0];
    float3 pointOnLight = UniformSampleRect(curLight, rndUV);

    float3 L = normalize(pointOnLight - wpos);
    float3 N = normalize(gbNormal[pixelIndex].xyz);

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    Ray ray;
    ray.d = float4(L, 0.0);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    float distToLight = distance(pointOnLight, wpos + offset);
    ray.o = float4(wpos + offset, distToLight - 1e-5);
    Hit hit;
    hit.t = 0.0;
    if ((dot(N, L) > 0.0) && anyHit(accel, ray, hit))
    {
        return 0.0;
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

