#pragma once

#define INVALID_INDEX 0xFFFFFFFF
#define PI 3.1415926535897

float3 interpolateAttrib(float3 attr1, float3 attr2, float3 attr3, float2 bary)
{
    return attr1 * (1 - bary.x - bary.y) + attr2 * bary.x + attr3 * bary.y;
}

float2 interpolateAttrib(float2 attr1, float2 attr2, float2 attr3, float2 bary)
{
    return attr1 * (1 - bary.x - bary.y) + attr2 * bary.x + attr3 * bary.y;
}

float3x3 GetTangentSpace(float3 normal)
{
    float3 helper = float3(1, 0, 0);
    if (abs(normal.x) > 0.99f)
        helper = float3(0, 0, 1);

    float3 tangent = normalize(cross(normal, helper));
    float3 binormal = normalize(cross(normal, tangent));
    return transpose(float3x3(tangent, binormal, normal));
}

float3 SampleHemisphere(uint2 pixelIndex, float3 normal)
{
    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float cosTheta = rand(rngState);
    float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));
    float phi = 2 * PI * rand(rngState);
    float3 tangentSpaceDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    return mul(GetTangentSpace(normal), tangentSpaceDir);
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
