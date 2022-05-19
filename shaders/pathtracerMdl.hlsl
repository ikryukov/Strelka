#include "materials.h"
#include "pack.h"
#include "lights.h"
#include "raytracing.h"
#include "pathtracerparam.h"
#include "instanceconstants.h"
// #include "mdl_types.hlsl"
// #include "mdl_runtime.hlsl"

ConstantBuffer<PathTracerParam> ubo;

// Texture2D<float4> gbWPos;
// Texture2D<float4> gbNormal;
// Texture2D<float4> gbTangent;
// Texture2D<int> gbInstId;
// Texture2D<float2> gbUV;

TextureCube<float4> cubeMap;
SamplerState cubeMapSampler;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<MdlMaterial> mdlMaterials;
StructuredBuffer<UniformLight> lights;

RWStructuredBuffer<float> sampleBuffer;
// RWTexture2D<float4> output;

float3 estimateDirectLighting(inout uint rngState,
                              in Accel accel,
                              in UniformLight light,
                              in Shading_state_material state,
                              out float3 toLight,
                              out float lightPdf)
{
    LightSampleData lightSampleData;
    switch (light.type)
    {
    case 0:
        lightSampleData = SampleRectLight(light, float2(rand(rngState), rand(rngState)), state.position);
        break;
    case 1:
        lightSampleData = SampleDiscLight(light, float2(rand(rngState), rand(rngState)), state.position);
        break;
    case 2:
        lightSampleData = SampleSphereLight(light, state.normal, state.position, float2(rand(rngState), rand(rngState)));
        break;
    }

    toLight = lightSampleData.L;
    float3 Li = light.color.rgb;

    if (dot(state.normal, lightSampleData.L) > 0.0f && -dot(lightSampleData.L, lightSampleData.normal) > 0.0 && all(Li))
    {
        Ray shadowRay;
        shadowRay.d = float4(lightSampleData.L, 0.0f);
        shadowRay.o = float4(offset_ray(state.position, state.geom_normal), lightSampleData.distToLight - 1e-4f); // need to set offset to fix self-collision

        Hit shadowHit;
        shadowHit.t = 0.0;
        float visibility = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

        lightPdf = lightSampleData.pdf;
        return visibility * Li * saturate(dot(state.normal, lightSampleData.L));
    }

    return float3(0.0f);
}

float3 sampleLights(
    inout uint rngState, in Accel accel, in Shading_state_material state, out float3 toLight, out float lightPdf)
{
    uint lightId = (uint)(ubo.numLights * rand(rngState));
    float lightSelectionPdf = 1.0f / ubo.numLights;
    UniformLight currLight = lights[lightId];
    float3 r = estimateDirectLighting(rngState, accel, currLight, state, toLight, lightPdf);
    lightPdf *= lightSelectionPdf;
    return r;
}

// https://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf
float2 stratifiedSamplingOptimized(int s, int N = 16, int p = 16, float a = 1.0f){
    int m = int(sqrt(N * a));
    int n = (N + m - 1) / m;

    s = permute(s, N, p * 0x51633e2d);

    int sx = permute(s % m, m, p * 0x68bc21eb);
    int sy = permute(s / m, n, p * 0x02e5be93);

    float jx = randfloat(s, p * 0x967a889b);
    float jy = randfloat(s, p * 0x368cc8b7);

    float2 r = { (sx + (sy + jx) / n) / m, (s + jy) / N};

    return r;
}

float2 stratifiedSampling1(uint s, inout uint rngState, uint N = 16)
{
    int m = int(sqrt(N));
    int n = (N + m - 1) / m;

    int x = (s / n) % n;
    int y = (s % m) % m;

    float2 jitter = float2(0.0f, 0.0f);
    jitter.x = (x + (y + rand(rngState)) / n) / m;
    jitter.y = (y + (x + rand(rngState)) / m) / n;

    return jitter;
}

float2 stratifiedSampling(uint s, in out uint rngState, uint N = 16)
{
    float2 p[16]; // [N]
    int m = int(sqrt(N));
    int n = (N + m - 1) / m;

    // Producing the canonical arrangement.
    for (int j = 0; j < n; ++j){
        for (int i = 0; i < m; ++i) {
            p[j * m + i].x = (i + (j + rand(rngState)) / n) / m;
            p[j * m + i].y = (j + (i + rand(rngState)) / m) / n;
        }
    }

    // Shuffling the canonical arrangement.
    for (int j = 0; j < n; ++j){
        for (int i = 0; i < m; ++i) {
            int k = j + rand(rngState) * (n - j);
            float tmp = p[j * m + i].x;
            p[j * m + i].x = p[k * m + i].x;
            p[k * m + i].x = tmp;

        }
    }

     for (int i = 0; i < m; ++i){
        for (int j = 0; j < n; ++j) {
            int k = i + rand(rngState) * (m - i);
            float tmp = p[j * m + i].y;
            p[j * m + i].y = p[j * m + k].y;
            p[j * m + k].y = tmp;
        }
     }

    return p[s % N];
}

// Matrices version
Ray generateCameraRay(uint2 pixelIndex, inout uint rngState, uint s)
{
    float2 pixelPos = float2(0.0f, 0.0f);

    if (ubo.stratifiedSamplingType == 0)
    {
        pixelPos = float2(pixelIndex) + 0.5f;
    }
    else if (ubo.stratifiedSamplingType == 1)
    {
        pixelPos.x = float2(pixelIndex).x + rand(rngState);
        pixelPos.y = float2(pixelIndex).y + rand(rngState);
    }
    else if (ubo.stratifiedSamplingType == 2)
    {
//         pixelPos = float2(pixelIndex) + stratifiedSampling(s, rngState);
        pixelPos = float2(pixelIndex) + stratifiedSampling1(s, rngState);
    }
    else if (ubo.stratifiedSamplingType == 3)
    {
        pixelPos = float2(pixelIndex) + stratifiedSamplingOptimized(s);
    }

    float2 pixelNDC = (pixelPos / float2(ubo.dimension)) * 2.0f - 1.0f;

    float4 clip = float4(pixelNDC, 1.0f, 1.0f);
    float4 viewSpace = mul(ubo.clipToView, clip);

    float4 wdir = mul(ubo.viewToWorld, float4(viewSpace.xyz, 0.0f));

    Ray ray = (Ray) 0;
    //ray.o = ubo.camPos; // mul to view to world
    ray.o = mul(ubo.viewToWorld, float4(0.0f, 0.0f, 0.0f, 1.0f));
    ray.o.w = 1e9f;
    ray.d.xyz = normalize(wdir.xyz);

    return ray;
}

float3 pathTraceCameraRays(uint2 pixelIndex, in out uint rngState, uint s)
{
    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    float3 finalColor = float3(0.0f);
    float3 throughput = float3(1.0f);

    int depth = 0;
    const int maxDepth = ubo.maxDepth;

    Ray ray = generateCameraRay(pixelIndex, rngState, s);

    bool inside = 0; // 1 - inside

    while (depth < maxDepth)
    {
        Hit hit;
        if (closestHit(accel, ray, hit))
        {
            InstanceConstants instConst = accel.instanceConstants[NonUniformResourceIndex(hit.instId)];

            if (instConst.lightId != -1)
            {
                UniformLight currLight = lights[instConst.lightId];
                finalColor += throughput * currLight.color.rgb;
                break;
            }
            else
            {
                uint i0 = accel.ib[NonUniformResourceIndex(instConst.indexOffset + hit.primId * 3 + 0)];
                uint i1 = accel.ib[NonUniformResourceIndex(instConst.indexOffset + hit.primId * 3 + 1)];
                uint i2 = accel.ib[NonUniformResourceIndex(instConst.indexOffset + hit.primId * 3 + 2)];

                float3 p0 = mul(instConst.objectToWorld, float4(accel.vb[NonUniformResourceIndex(i0)].position, 1.0f)).xyz;
                float3 p1 = mul(instConst.objectToWorld, float4(accel.vb[NonUniformResourceIndex(i1)].position, 1.0f)).xyz;
                float3 p2 = mul(instConst.objectToWorld, float4(accel.vb[NonUniformResourceIndex(i2)].position, 1.0f)).xyz;

                float3 geomNormal = normalize(cross(p1 - p0, p2 - p0));

                float3 n0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i0)].normal));
                float3 n1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i1)].normal));
                float3 n2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i2)].normal));

                float3 t0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i0)].tangent));
                float3 t1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i1)].tangent));
                float3 t2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i2)].tangent));

                const float2 bcoords = hit.bary;
                float3 worldPosition = interpolateAttrib(p0, p1, p2, bcoords);
                float3 worldNormal = normalize(interpolateAttrib(n0, n1, n2, bcoords));
                float3 worldTangent = normalize(interpolateAttrib(t0, t1, t2, bcoords));
                geomNormal *= (inside ? -1.0 : 1.0);
                worldNormal *= (inside ? -1.0 : 1.0);
                float3 worldBinormal = cross(worldNormal, worldTangent);

                float2 uv0 = unpackUV(accel.vb[NonUniformResourceIndex(i0)].uv);
                float2 uv1 = unpackUV(accel.vb[NonUniformResourceIndex(i1)].uv);
                float2 uv2 = unpackUV(accel.vb[NonUniformResourceIndex(i2)].uv);

                float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

                if (ubo.debug == 1)
                {
                    float3 debugN = (worldNormal + 1.0) * 0.5;
                    // float3 debugN = (worldNormal);
                    return debugN;
                }

                MdlMaterial currMdlMaterial = mdlMaterials[NonUniformResourceIndex(instConst.materialId)];

                // setup MDL state
                Shading_state_material mdlState;
                mdlState.normal = worldNormal;
                mdlState.geom_normal = geomNormal;
                mdlState.position = worldPosition; // hit position
                mdlState.animation_time = 0.0f;
                mdlState.tangent_u[0] = worldTangent;
                mdlState.tangent_v[0] = worldBinormal;
                mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
                mdlState.world_to_object = instConst.worldToObject;
                mdlState.object_to_world = instConst.objectToWorld;
                mdlState.object_id = hit.instId;
                mdlState.meters_per_scene_unit = 1.0f;
                mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
                mdlState.text_coords[0] = float3(uvCoord, 0);

                float3 shadingNormal = mdlState.geom_normal;

                const float ior1 = (inside) ? BSDF_USE_MATERIAL_IOR : 1.0f; // material -> air
                const float ior2 = (inside) ? 1.0f : BSDF_USE_MATERIAL_IOR;

                int scatteringFunctionIndex = currMdlMaterial.functionId;

                Edf_evaluate_data edfEval = (Edf_evaluate_data) 0;
                edfEval.k1 = -ray.d.xyz;
                mdl_edf_emission_init(scatteringFunctionIndex, mdlState);
                mdl_edf_emission_evaluate(scatteringFunctionIndex, edfEval, mdlState);
                float3 intensity = mdl_edf_emission_intensity(scatteringFunctionIndex, mdlState);
                finalColor += throughput * intensity * edfEval.edf;

                mdlState.geom_normal = shadingNormal; // reset normal (init calls can change the normal due to maps)
                mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

                float3 toLight; //return value for sampleLights()
                float lightPdf = 0.0f; //return value for sampleLights()

                float3 radiance = sampleLights(rngState, accel, mdlState, toLight, lightPdf);

                if (any(isnan(radiance)) || isnan(lightPdf))
                {
                    break;
                }

                const bool nextEventValid = ((dot(toLight, mdlState.geom_normal) > 0.0f) != inside) && lightPdf != 0.0f;
                //const bool nextEventValid = true;
                if (nextEventValid)
                {
                    float3 radianceOverPdf = radiance / lightPdf;
                    Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;

                    evalData.ior1 = ior1;       // IOR current medium
                    evalData.ior2 = ior2;       // IOR other side
                    evalData.k1 = -ray.d.xyz;  // outgoing direction
                    evalData.k2 = toLight;     // incoming direction

                    mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

                    if (any(isnan(evalData.bsdf_diffuse)) || any(isnan(evalData.bsdf_glossy)) )
                    {
                        break;
                    }

                    // compute lighting for this light
                    if (evalData.pdf > 0.0f)
                    {
                        const float misWeight = (lightPdf == 0.0f) ? 1.0f : lightPdf / (lightPdf + evalData.pdf);
                        const float3 w = throughput * radianceOverPdf * misWeight;
                        finalColor += w * evalData.bsdf_diffuse;
                        finalColor += w * evalData.bsdf_glossy;
                    }
                }

                float4 rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

                Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
                sampleData.ior1 = ior1;
                sampleData.ior2 = ior2;
                sampleData.k1 = -ray.d.xyz;
                sampleData.xi = rndSample;

                mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

                if (sampleData.event_type == BSDF_EVENT_ABSORB)
                {
                    // stop on absorb
                     break;
                }

                // flip inside/outside on transmission
                // setup next path segment
                inside = ((sampleData.event_type & BSDF_EVENT_TRANSMISSION) != 0);
                float3 rayDirectionNext = sampleData.k2;
                float3 rayOriginNext = offset_ray(mdlState.position, mdlState.geom_normal * (inside ? -1.0 : 1.0));

                throughput *= sampleData.bsdf_over_pdf;

                if (depth > 3)
                {
                    float p = max(throughput.r, max(throughput.g, throughput.b));
                    if (rand(rngState) > p)
                    {
                        break;
                    }
                    throughput *= 1.0 / (p + 1e-5);
                }

                // add check and flip offset for transmission event
                ray.o = float4(rayOriginNext, 1e9);
                ray.d = float4(rayDirectionNext, 0.0f);
            }
        }
        else
        {
            // miss - add background color and exit
            // MIS weight for non-specular BSDF events
            //float3 viewSpaceDir = mul((float3x3) ubo.worldToView, ray.d.xyz);
            //finalColor += throughput * cubeMap.Sample(cubeMapSampler, viewSpaceDir).rgb;

            finalColor += throughput * float3(1.f);

            break;
        }
        ++depth;
    }

    return finalColor;
}

[numthreads(256, 1, 1)]
[shader("compute")]
void computeMain(uint2 dispatchIndex : SV_DispatchThreadID)
{
    if (dispatchIndex.x >= ubo.dimension.x * ubo.dimension.y * ubo.spp)
    {
        return;
    }
    uint2 pixelIndex = uint2(dispatchIndex.x / ubo.spp % ubo.dimension.x, dispatchIndex.x / ubo.spp / ubo.dimension.x);
    uint sampleNum = dispatchIndex.x % ubo.spp;

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber * ubo.spp + sampleNum);
    float3 color = pathTraceCameraRays(pixelIndex, rngState, sampleNum);

    sampleBuffer[dispatchIndex.x * 3 + 0] = color.r;
    sampleBuffer[dispatchIndex.x * 3 + 1] = color.g;
    sampleBuffer[dispatchIndex.x * 3 + 2] = color.b;
}
