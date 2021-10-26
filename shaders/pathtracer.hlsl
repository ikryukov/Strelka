#include "helper.h"
#include "materials.h"
#include "pack.h"
#include "lights.h"
#include "helper.h"
#include "raytracing.h"
#include "pathtracerparam.h"

ConstantBuffer<PathTracerParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<int> gbInstId;
Texture2D<float2> gbUV;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;
StructuredBuffer<RectLight> lights;

Texture2D textures[BINDLESS_TEXTURE_COUNT];
SamplerState samplers[BINDLESS_SAMPLER_COUNT];

RWTexture2D<float4> output;

float3 UniformSampleRect(RectLight l, float2 u)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return l.points[0].xyz + e1 * u.x + e2 * u.y;
}

float3 calcLightNormal(RectLight l)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return normalize(cross(e1, e2));
}

float calcLightArea(RectLight l)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[2].xyz - l.points[1].xyz;
    return length(e1) * length(e2);    
}

float3 pathTrace(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    // early out - miss on camera ray
    if (gbWorldPos.w == 0.0)
        return 0;

    InstanceConstants instConst = instanceConstants[gbInstId[pixelIndex]];
    Material material = materials[instConst.materialId];
    float2 matUV = gbUV[pixelIndex];
    if (material.isLight)
    {
        return getBaseColor(material, matUV, textures, samplers);
    }
    float3 wpos = gbWPos[pixelIndex].xyz;

    float3 N = normalize(gbNormal[pixelIndex].xyz);
    float3x3 TBN = GetTangentSpace(N);
    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    Ray ray;
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    ray.o = float4(wpos + offset, 1e9);
    Hit hit;
    hit.t = 0.0;

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    // calc camera ray
    RectLight currLight = lights[0]; // TODO: sample lights
    const float3 pointOnLight = UniformSampleRect(currLight, float2(rand(rngState), rand(rngState)));
    float3 L = normalize(pointOnLight - ray.o.xyz);
    float distToLight = distance(pointOnLight, ray.o.xyz);
    float3 lightNormal = calcLightNormal(currLight);
    float lightArea = calcLightArea(currLight);
    float lightPDF = distToLight * distToLight / (-dot(L, lightNormal) * lightArea);
    Ray shadowRay;
    shadowRay.d = float4(L, 0.0);
    shadowRay.o = float4(ray.o.xyz, distToLight - 1e-5);

    Hit shadowHit;
    shadowHit.t = 0.0;
    float shadow = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

    float3 diffuse = getBaseColor(material, matUV, textures, samplers);
    float3 materialBsdf = 1.0f / PI * diffuse;
    float materialBsdfPdf = 1.0f / (2.0f * PI);
    float3 finalColor =  shadow * currLight.color.rgb * materialBsdf * saturate(dot(N, L)) / lightPDF;
    
    int depth = 1;
    int maxDepth = ubo.maxDepth;
    
    // generate new ray
    TBN = GetTangentSpace(N); // N - hit normal
    float3 tangentSpaceDir = SampleHemisphere(rand(rngState), rand(rngState), 0.0); // 0 - uniform sampling, 1 - cos. sampling, higher for phong
    float3 dir = mul(TBN, tangentSpaceDir);
    
    ray.d = float4(dir, 0.0);

    float3 throughput = materialBsdf * dot(N, ray.d.xyz) / materialBsdfPdf;
    
    while (depth < maxDepth)
    {
            if (closestHit(accel, ray, hit))
            {
                instConst = accel.instanceConstants[hit.instId];
                material = materials[instConst.materialId];

                uint i0 = accel.ib[instConst.indexOffset + hit.primId * 3 + 0];
                uint i1 = accel.ib[instConst.indexOffset + hit.primId * 3 + 1];
                uint i2 = accel.ib[instConst.indexOffset + hit.primId * 3 + 2];

                float3 n0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].normal));
                float3 n1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].normal));
                float3 n2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].normal));

                float2 bcoords = hit.bary;
                float3 n = normalize(interpolateAttrib(n0, n1, n2, bcoords));
                N = n; // hit normal

                float2 uv0 = unpackUV(accel.vb[i0].uv);
                float2 uv1 = unpackUV(accel.vb[i1].uv);
                float2 uv2 = unpackUV(accel.vb[i2].uv);

                float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

                float3 diffuse = getBaseColor(material, uvCoord, textures, samplers);

                if (material.isLight)
                {
                    finalColor += throughput * diffuse;
                    //break;
                    depth = maxDepth;
                }
                else
                {
                    RectLight currLight = lights[0]; // TODO: sample lights
                    const float3 pointOnLight = UniformSampleRect(currLight, float2(rand(rngState), rand(rngState)));
                    float3 L = normalize(pointOnLight - ray.o.xyz);
                    float distToLight = distance(pointOnLight, ray.o.xyz);
                    float3 lightNormal = calcLightNormal(currLight);
                    float lightArea = calcLightArea(currLight);
                    float lightPDF = distToLight * distToLight / (-dot(L, lightNormal) * lightArea);
                    Ray shadowRay;
                    shadowRay.d = float4(L, 0.0);
                    shadowRay.o = float4(ray.o.xyz, distToLight - 1e-5);

                    Hit shadowHit;
                    shadowHit.t = 0.0;
                    float shadow = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

                    float3 materialBsdf = 1.0f / PI * diffuse;
                    float materialBsdfPdf = 1.0f / (2.0f * PI);
                    float3 directLight =  shadow * currLight.color.rgb * materialBsdf * saturate(dot(N, L)) / lightPDF;

                    finalColor += throughput * directLight;

                    // generate new ray
                    TBN = GetTangentSpace(N); // N - hit normal
                    float3 tangentSpaceDir = SampleHemisphere(rand(rngState), rand(rngState), 0.0); // 0 - uniform sampling, 1 - cos. sampling, higher for phong
                    float3 dir = mul(TBN, tangentSpaceDir);
                    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
                    ray.o = float4(ray.o.xyz + ray.d.xyz * hit.t + offset, 1e9); // new ray origin for next ray
                    ray.d = float4(dir, 0.0);

                    throughput *= materialBsdf * dot(N, ray.d.xyz) / materialBsdfPdf;
                }
            }
            else
            {
                // miss - add background color and exit
                finalColor += throughput * float3(0.f);
                //break;
                depth = maxDepth;
            }
            depth += 1;
    }

    return finalColor;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = 0.f;

    color = pathTrace(pixelIndex);

    output[pixelIndex] = float4(color, 1.0);
}