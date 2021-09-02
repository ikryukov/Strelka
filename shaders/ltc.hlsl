#include "random.h"
#include "materials.h"
#include "lights.h"

cbuffer ubo
{
    float4x4 viewToProj;
    float4x4 worldToView;
    float3 CameraPos;
    uint frameNumber;
    uint2 dimension;
    float pad0;
    float pad1;
}

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<RectLight> lights;
StructuredBuffer<Material> materials;

Texture2D<float4> ltc1;
Texture2D<float4> ltc2;
SamplerState ltcSampler;

RWTexture2D<float> output;

// source: https://blog.selfshadow.com/ltc/webgl/ltc_quad.html
// Linearly Transformed Cosines
///////////////////////////////
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * rsqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

float3 LTC_Evaluate(RectLight light, float3 N, float3 V, float3 P, float3x3 Minv, bool twoSided)
{
    // construct orthonormal basis around N
    float3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = normalize(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = mul(transpose(float3x3(T1, T2, N)), Minv);

    float3 L[4];
    L[0] = mul(light.points[0].xyz - P, Minv);
    L[1] = mul(light.points[1].xyz - P, Minv);
    L[2] = mul(light.points[2].xyz - P, Minv);
    L[3] = mul(light.points[3].xyz - P, Minv);

    // integrate
    float sum = 0.0;

    float3 dir = normalize(light.points[0].xyz - P);
    float3 lightNormal = cross(light.points[1] - light.points[0], light.points[3] - light.points[0]);
    bool behind = (dot(dir, lightNormal) > 0.0);

    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    float3 vsum = float3(0.0);

    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    float len = length(vsum);
    float z = vsum.z / len;

    if (!behind)
    {
        z = -z;
    }

    float2 uv = float2(z * 0.5 + 0.5, len);
    uv = uv * LUT_SCALE + LUT_BIAS;

    float scale = ltc2.SampleLevel(ltcSampler, uv, 0).w;
    sum = len * scale;

    if (behind && !twoSided)
    {
        sum = 0.0;
    }

    float3 Lo_i = vec3(sum, sum, sum);
    return Lo_i;
}

float3 calc(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWorldPos.xyz;
    int instId = gbInstId[pixelIndex];
    InstanceConstants constants = instanceConstants[NonUniformResourceIndex(instId)];
    Material material = materials[NonUniformResourceIndex(constants.materialId)];
    RectLight light = lights[0];

    float3 N = normalize(gbNormal[pixelIndex].xyz);
    float3 V = normalize(CameraPos - wpos);
    float ndotv = saturate(dot(N, V));
    float roughness = material.roughnessFactor;
    float2 uv = float2(roughness, sqrt(1.0 - ndotv));
    uv = uv * LUT_SCALE + LUT_BIAS;

    float4 t1 = ltc1.SampleLevel(ltcSampler, uv, 0);
    float4 t1 = ltc2.SampleLevel(ltcSampler, uv, 0);

    float3x3 Minv = float3x3(
        float3(t1.x, 0, t1.y), 
        float3(   0, 1,    0), 
        float3(t1.z, 0, t1.w)
    );

    float3 res = LTC_Evaluate(light, N, V, wpos, Minv, twoSided);
    return res;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= dimension.x || pixelIndex.y >= dimension.y)
    {
        return;
    }
    output[pixelIndex] = float4(calc(pixelIndex), 1.0);
}
