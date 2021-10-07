#include "aopassparam.h"
#include "raytracing.h"

ConstantBuffer<AOParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<BVHTriangle> bvhTriangles;

RWTexture2D<float> output;

float3x3 GetTangentSpace(uint2 pixelIndex)
{
    float3 normal = gbNormal[pixelIndex].xyz;
    float3 helper = float3(1, 0, 0);
    if (abs(normal.x) > 0.99f)
        helper = float3(0, 0, 1);

    float3 tangent = normalize(cross(normal, helper));
    float3 binormal = normalize(cross(normal, tangent));
    return transpose(float3x3(tangent, binormal, normal));
}

float3 SampleHemisphere(uint2 pixelIndex)
{
    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float cosTheta = rand(rngState);
    float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));
    float phi = 2 * PI * rand(rngState);
    float3 tangentSpaceDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    return mul(GetTangentSpace(pixelIndex), tangentSpaceDir);
}

float3 SampleHemisphere(float u1, float u2, float alpha)
{
    float cosTheta = pow(u1, 1.0f / (alpha + 1.0f));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = 2 * PI * u2;
    float3 tangentSpaceDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    return tangentSpaceDir;
}

// Returns a random direction on the hemisphere around z = 1
// Uniform
float3 SampleDirectionHemisphere(float u1, float u2)
{
    float z = u1;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2 * PI * u2;
    float x = r * cos(phi);
    float y = r * sin(phi);
    return float3(x, y, z);
}

float calcAO(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWPos[pixelIndex].xyz;
    float3 N = normalize(gbNormal[pixelIndex].xyz);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    Ray ray;
    ray.o = float4(wpos + offset, ubo.rayLen);
    float3x3 TBN = GetTangentSpace(pixelIndex);
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
        if (anyHit(ray, hit))
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
