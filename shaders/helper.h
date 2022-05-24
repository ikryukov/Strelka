#pragma once

#define PI 3.1415926535897
#define INVERSE_PI (1.0 / PI)
#define PiOver2 1.57079632679489661923
#define PiOver4 0.78539816339744830961
#define DIRAC -1.0f

// Clever offset_ray function from Ray Tracing Gems chapter 6
// Offsets the ray origin from current position p, along normal n (which must be geometric normal)
// so that no self-intersection can occur.
float3 offset_ray(const float3 p, const float3 n)
{
    static const float origin = 1.0f / 32.0f;
    static const float float_scale = 1.0f / 65536.0f;
    static const float int_scale = 256.0f;

    int3 of_i = int3(int_scale * n.x, int_scale * n.y, int_scale * n.z);

    float3 p_i = float3(
        asfloat(asint(p.x) + ((p.x < 0) ? -of_i.x : of_i.x)),
        asfloat(asint(p.y) + ((p.y < 0) ? -of_i.y : of_i.y)),
        asfloat(asint(p.z) + ((p.z < 0) ? -of_i.z : of_i.z)));

    return float3(abs(p.x) < origin ? p.x + float_scale * n.x : p_i.x,
                  abs(p.y) < origin ? p.y + float_scale * n.y : p_i.y,
                  abs(p.z) < origin ? p.z + float_scale * n.z : p_i.z);
}


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
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
    float phi = 2 * PI * rand(rngState);
    float3 tangentSpaceDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    return mul(GetTangentSpace(normal), tangentSpaceDir);
}

float3 SampleRayInHemisphere(const float3 hitPoint, const float2 u)
{
    float signZ = (hitPoint.z >= 0.0f) ? 1.0f : -1.0f;
    float a = -1.0f / (signZ + hitPoint.z);
    float b = hitPoint.x * hitPoint.y * a;
    float3 b1 = float3(1.0f + signZ * hitPoint.x * hitPoint.x * a, signZ * b, -signZ * hitPoint.x);
    float3 b2 = float3(b, signZ + hitPoint.y * hitPoint.y * a, -hitPoint.y);

    float phi = 2.0f * PI * u.x;
    float cosTheta = sqrt(u.y);
    float sinTheta = sqrt(1.0f - u.y);
    return normalize((b1 * cos(phi) + b2 * sin(phi)) * cosTheta + hitPoint * sinTheta);
}


float3 SampleHemisphere(float u1, float u2, float alpha)
{
    float cosTheta = pow(u1, 1.0f / (alpha + 1.0f));
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
    float phi = 2 * PI * u2;
    float3 tangentSpaceDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    return tangentSpaceDir;
}

float3 SampleHemisphereCosine(in float2 uv)
{
    float cosTheta = sqrt(saturate(uv.x));
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
    float phi = 2 * PI * uv.y;
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta + 1e-6);
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

float3 SampleGGXDistribution(float2 uv, float alpha)
{
    float cosTheta = sqrt((1.0f - uv.x) / (uv.x * (alpha * alpha - 1.0f) + 1.0f));
    float sinTheta = sqrt(saturate(1.0f - cosTheta * cosTheta));
    float phi = 2 * PI * uv.y;
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta + 1e-6);
}

float3 CalcBumpedNormal(float3 normal, float3 tangent, float2 uv, uint32_t texId, uint32_t sampId)
{
    float3 Normal = normalize(normal);
    float3 Tangent = -normalize(tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    float3 Bitangent = cross(Normal, Tangent);

    float3 BumpMapNormal = mdl_textures_2d[NonUniformResourceIndex(texId)].Sample(mdl_sampler_tex, uv).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;

    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    float3 NewNormal = normalize(mul(TBN, BumpMapNormal));

    return NewNormal;
}

uint permute(uint i, uint l, uint p) {
    uint w = l - 1;

    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;

    do {
        i ^= p;
        i *= 0xe170893d;
        i ^= p >> 16;
        i ^= (i & w) >> 4;
        i ^= p >> 8;
        i *= 0x0929eb3f;
        i ^= p >> 23;
        i ^= (i & w) >> 1;
        i *= 1 | p >> 27;
        i *= 0x6935fa69;
        i ^= (i & w) >> 11;
        i *= 0x74dcb303;
        i ^= (i & w) >> 2;
        i *= 0x9e501cc3;
        i ^= (i & w) >> 2;
        i *= 0xc860a3df;
        i &= w;
        i ^= i >> 5;
    } while (i >= l);

    return (i + p) % l;
}

float randfloat(uint i, uint p) {
    i ^= p;
    i ^= i >> 17;
    i ^= i >> 10;
    i *= 0xb36534e5;
    i ^= i >> 12;
    i ^= i >> 21;
    i *= 0x93fc4795;
    i ^= 0xdf6e307f;
    i ^= i >> 17;
    i *= 1 | p >> 18;

    return i * (1.0f / 4294967808.0f);
}
