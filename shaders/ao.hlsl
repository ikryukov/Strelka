#include "aopassparam.h"
#include "raytracing.h"
#include "helper.h"

ConstantBuffer<AOParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvhNodes;

StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;

RWTexture2D<float> output;

float calcAO(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWPos[pixelIndex].xyz;
    float3 N = normalize(gbNormal[pixelIndex].xyz);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    
    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    Ray ray;
    ray.o = float4(wpos + offset, ubo.rayLen);
    float3x3 TBN = GetTangentSpace(N);
    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);
    float res = 0.0;
    for (int i = 0; i < ubo.samples; ++i)
    {
        float u1 = rand(rngState);
        float u2 = rand(rngState);
        float3 tangentSpaceDir = SampleHemisphere(u1, u2, 1.0); // 0 - uniform sampling, 1 - cos. sampling, higher for phong
        const float3 dir = mul(TBN, tangentSpaceDir);
        ray.d = float4(dir, 0.0);        

        Hit hit;
        hit.t = 0.0;
        if (anyHit(accel, ray, hit))
        {
            res += 0.0;
        }
        else
        {
            res += 1.0;
        }
    }

   return res / ubo.samples;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    output[pixelIndex] = calcAO(pixelIndex);
}
