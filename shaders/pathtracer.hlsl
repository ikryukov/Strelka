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

struct SampledMaterial
{
    float3 diffuse;
    float3 wInp;
    float3 wOut;
    float bsdf;
    float pdf;
};

struct SurfacePoint
{
    float3 p;
    float3 N;
}

float3 UniformSampleRect(in RectLight l, float2 u)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return l.points[0].xyz + e1 * u.x + e2 * u.y;
}

float3 calcLightNormal(in RectLight l)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return normalize(cross(e1, e2));
}

float calcLightArea(in RectLight l)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return length(cross(e1, e2));    
}

float3 estimateDirectLighting(inout uint rngState, in Accel accel, in RectLight light, in SurfacePoint hit, in SampledMaterial material)
{
    const float3 pointOnLight = UniformSampleRect(light, float2(rand(rngState), rand(rngState)));
    float3 L = normalize(pointOnLight - hit.p);
    float3 lightNormal = calcLightNormal(light);
    float3 Li = light.color.rgb;

    if (dot(hit.N, L) > 0 && -dot(L, lightNormal) > 0.0 && all(Li))
    {
        float distToLight = distance(pointOnLight, hit.p);
        float lightArea = calcLightArea(light);
        float lightPDF = distToLight * distToLight / (-dot(L, lightNormal) * lightArea);

        Ray shadowRay;
        shadowRay.d = float4(L, 0.0);
        const float3 offset = hit.N * 1e-6; // need to add small offset to fix self-collision
        shadowRay.o = float4(hit.p + offset, distToLight - 1e-5);

        Hit shadowHit;
        shadowHit.t = 0.0;
        float visibility = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

        return visibility * Li * material.diffuse * material.bsdf * saturate(dot(hit.N, L)) / lightPDF;
    }

    return float3(0.0);
}

float3 sampleLights(inout uint rngState, in Accel accel, in SurfacePoint hit, in SampledMaterial material)
{
    RectLight currLight = lights[0]; // TODO: sample lights
    return estimateDirectLighting(rngState, accel, currLight, hit, material);
}

SampledMaterial sampleMaterial(in Material material, float2 uv, float3 N, float3 wi, float3 wo)
{
    SampledMaterial sm;
    sm.diffuse = getBaseColor(material, uv, textures, samplers);
    sm.bsdf = 1.0f / PI;
    //sm.pdf = 1.0f / (2.0f * PI); // 1/2pi - uniform, cos(phi)/pi - cosine
    sm.pdf = 1.0f / PI * saturate(dot(N, wo));
    sm.wInp = wi;
    sm.wOut = wo;
    return sm;
}

SampledMaterial sampleMaterial(in Material material, float2 uv, float3 N, float3 wi, float2 noiseSample)
{
    float3x3 TBN = GetTangentSpace(N);
    float3 wOut = SampleHemisphereCosine(noiseSample);
    wOut = mul(TBN, wOut);
    return sampleMaterial(material, uv, N, wi, wOut);
}

float3 CalcBumpedNormal(float3 normal, float3 tangent, float2 uv, uint32_t texId, uint32_t sampId)
{
    float3 Normal = normalize(normal);
    float3 Tangent = -normalize(tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    float3 Bitangent = cross(Normal, Tangent);

    float3 BumpMapNormal = textures[NonUniformResourceIndex(texId)].Sample(samplers[sampId], uv).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;

    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    float3 NewNormal = normalize(mul(TBN, BumpMapNormal));

    return NewNormal;
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
    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    // calc camera ray
    SurfacePoint sp;
    sp.N = N;
    sp.p = wpos;

    float2 rndSample = float2(rand(rngState), rand(rngState));

    SampledMaterial sm = sampleMaterial(material, matUV, N, float3(1.0), rndSample);

    float3 finalColor = sampleLights(rngState, accel, sp, sm);
    
    int depth = 1;
    int maxDepth = ubo.maxDepth;
    
    // generate new ray
    Ray ray;
    const float3 offset = N * 1e-6; // need to add small offset to fix self-collision
    ray.o = float4(sp.p + offset, 1e9);
    ray.d = float4(sm.wOut, 0.0);

    Hit hit;
    hit.t = 0.0;

    float3 throughput = sm.diffuse * sm.bsdf * dot(sp.N, ray.d.xyz) / sm.pdf;
    
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

            float2 uv0 = unpackUV(accel.vb[i0].uv);
            float2 uv1 = unpackUV(accel.vb[i1].uv);
            float2 uv2 = unpackUV(accel.vb[i2].uv);

            float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

            if (material.texNormalId != INVALID_INDEX)
            {
                float3 t0 = unpackTangent(accel.vb[i0].tangent);
                float3 t1 = unpackTangent(accel.vb[i1].tangent);
                float3 t2 = unpackTangent(accel.vb[i2].tangent);

                float3 t = normalize(interpolateAttrib(t0, t1, t2, bcoords));

                n = CalcBumpedNormal(n, t, uvCoord,  material.texNormalId, material.sampNormalId);
            }

            N = n; // hit normal

            rndSample = float2(rand(rngState), rand(rngState));
            sm = sampleMaterial(material, uvCoord, N, ray.d.xyz, rndSample);

            if (material.isLight)
            {
                finalColor += throughput * sm.diffuse;
                //break;
                depth = maxDepth;
            }
            else
            {
                sp.p = ray.o.xyz + ray.d.xyz * hit.t;
                sp.N = N;

                finalColor += throughput * sampleLights(rngState, accel, sp, sm);

                // generate new ray
                const float3 offset = N * 1e-6; // need to add small offset to fix self-collision
                ray.o = float4(sp.p + offset, 1e9); // new ray origin for next ray
                ray.d = float4(sm.wOut, 0.0);

                throughput *= sm.diffuse * sm.bsdf * dot(N, ray.d.xyz) / sm.pdf;

                // Russian Roulette
                if (depth > 3)
                {
                    float p = max(throughput.r, max(throughput.g, throughput.b));
                    if (rand(rngState) > p)
                    {
                        // break
                        depth = maxDepth;
                    }
                    throughput *= 1.0 / p;
                }
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
