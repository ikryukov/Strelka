#include "helper.h"
#include "materials.h"
#include "pack.h"
#include "lights.h"
#include "raytracing.h"
#include "pathtracerparam.h"
#include "instanceconstants.h"
// #include "mdl_types.hlsl"
// #include "mdl_runtime.hlsl"

ConstantBuffer<PathTracerParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> gbTangent;
Texture2D<int> gbInstId;
Texture2D<float2> gbUV;

TextureCube<float4> cubeMap;
SamplerState cubeMapSampler;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;
StructuredBuffer<MdlMaterial> mdlMaterials;
StructuredBuffer<RectLight> lights;

RWStructuredBuffer<float> sampleBuffer;
// RWTexture2D<float4> output;

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

float3 estimateDirectLighting(inout uint rngState, in Accel accel, in RectLight light, in Shading_state_material state, out float3 toLight, out float lightPdf)
{
    const float3 pointOnLight = UniformSampleRect(light, float2(rand(rngState), rand(rngState)));
    float3 L = normalize(pointOnLight - state.position);
    toLight = L;
    float3 lightNormal = calcLightNormal(light);
    float3 Li = light.color.rgb;

    if (dot(state.normal, L) > 0 && -dot(L, lightNormal) > 0.0 && all(Li))
    {
        float distToLight = distance(pointOnLight, state.position);
        float lightArea = calcLightArea(light);
        float lightPDF = distToLight * distToLight / (-dot(L, lightNormal) * lightArea);

        Ray shadowRay;
        shadowRay.d = float4(L, 0.0);
        const float3 offset = state.normal * 1e-6f; // need to add small offset to fix self-collision
        shadowRay.o = float4(state.position + offset, distToLight - 1e-5);

        Hit shadowHit;
        shadowHit.t = 0.0;
        float visibility = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

        lightPdf = lightPDF;
        return visibility * Li * saturate(dot(state.normal, L)) / (lightPDF + 1e-5);
    }

    return float3(0.0);
}

float3 sampleLights(inout uint rngState, in Accel accel, in Shading_state_material state, out float3 toLight, out float lightPdf)
{
    uint lightId = (uint) (ubo.numLights * rand(rngState));
    float lightSelectionPdf = 1.0f / (ubo.numLights + 1e-6);
    RectLight currLight = lights[lightId];
    float3 r = estimateDirectLighting(rngState, accel, currLight, state, toLight, lightPdf);
    lightPdf *= lightSelectionPdf;

    return r / (lightPdf + 1e-5);
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

// Matrices version
Ray generateCameraRay(uint2 pixelIndex)
{
    float2 pixelPos = float2(pixelIndex) + 0.5f;
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

float3 pathTraceCameraRays(uint2 pixelIndex, in out uint rngState)
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

    Ray ray = generateCameraRay(pixelIndex);
    
    while (depth < maxDepth)
    {
        Hit hit;
        if (closestHit(accel, ray, hit))
        {
            InstanceConstants instConst = accel.instanceConstants[NonUniformResourceIndex(hit.instId)];
            Material material = materials[NonUniformResourceIndex(instConst.materialId)];

            if (material.isLight != -1)
            {
                // finalColor += throughput * float3(1.0f);
                RectLight currLight = lights[material.isLight];
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

                float3 geom_normal = normalize(cross(p1 - p0, p2 - p0));

                float3 n0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i0)].normal));
                float3 n1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i1)].normal));
                float3 n2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i2)].normal));

                float3 t0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i0)].tangent));
                float3 t1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i1)].tangent));
                float3 t2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[NonUniformResourceIndex(i2)].tangent));

                const float2 bcoords = hit.bary;
                float3 world_position = interpolateAttrib(p0, p1, p2, bcoords);
                float3 world_normal = normalize(interpolateAttrib(n0, n1, n2, bcoords));
                float3 world_tangent = normalize(interpolateAttrib(t0, t1, t2, bcoords));
                float3 world_binormal = cross(world_normal, world_tangent);

                float2 uv0 = unpackUV(accel.vb[NonUniformResourceIndex(i0)].uv);
                float2 uv1 = unpackUV(accel.vb[NonUniformResourceIndex(i1)].uv);
                float2 uv2 = unpackUV(accel.vb[NonUniformResourceIndex(i2)].uv);

                float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

                if (ubo.debug == 1)
                {
                    // float3 debugN = (world_normal + 1.0) * 0.5;
                    float3 debugN = (world_normal);
                    return debugN;
                }

                MdlMaterial currMdlMaterial = mdlMaterials[NonUniformResourceIndex(instConst.materialId)];

                // setup MDL state
                Shading_state_material mdlState;
                mdlState.normal = world_normal;
                mdlState.geom_normal = geom_normal;
                mdlState.position = world_position; // hit position
                mdlState.animation_time = 0.0f;
                mdlState.tangent_u[0] = world_tangent;
                mdlState.tangent_v[0] = world_binormal;
                mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
                mdlState.world_to_object = instConst.worldToObject;
                mdlState.object_to_world = instConst.objectToWorld;
                mdlState.object_id = 0;
                mdlState.meters_per_scene_unit = 1.0f;
                mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
                mdlState.text_coords[0] = float3(uvCoord, 0);

                int scatteringFunctionIndex = currMdlMaterial.functionId;
                mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

                float3 toLight; //return value for sampleLights()
                float lightPdf = 0.0f; //return value for sampleLights()
                float3 radianceOverPdf = sampleLights(rngState, accel, mdlState, toLight, lightPdf);
                
                if (any(isnan(radianceOverPdf)) || isnan(lightPdf))
                {
                    break;
                }

                Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;
                evalData.ior1 = 1.5;       // IOR current medium // 1.2
                evalData.ior2 = 1.5;       // IOR other side
                evalData.k1 = -ray.d.xyz;  // outgoing direction
                evalData.k2 = toLight;     // incoming direction

                mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

                if (any(isnan(evalData.bsdf_diffuse)) || any(isnan(evalData.bsdf_glossy)) )
                {
                    break;
                }

                if (evalData.pdf > 0.0f)
                {
                    const float mis_weight = lightPdf / (lightPdf + evalData.pdf);
                    const float3 w = throughput * radianceOverPdf * mis_weight;
                    finalColor += w * evalData.bsdf_diffuse;
                    finalColor += w * evalData.bsdf_glossy;
                }

                float4 rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

                Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
                sampleData.ior1 = 1.5;
                sampleData.ior2 = 1.5;
                sampleData.k1 = -ray.d.xyz;
                sampleData.xi = rndSample;

                mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

                if (sampleData.event_type == BSDF_EVENT_ABSORB)
                {
                    // stop on absorb
                    break;
                }

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

                const float3 offset = world_normal * 1e-5; // need to add small offset to fix self-collision
                // add check and flip offset for transmission event
                ray.o = float4(mdlState.position + offset, 1e9);
                ray.d = float4(sampleData.k2, 0.0);
            }
        }
        else
        {
            // miss - add background color and exit
            float3 viewSpaceDir = mul((float3x3) ubo.worldToView, ray.d.xyz);
            finalColor += throughput * cubeMap.Sample(cubeMapSampler, viewSpaceDir).rgb;

            break;
        }
        ++depth;
    }
    return finalColor;
}

float3 pathTraceGBuffer(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    // early out - miss on camera ray
    if (gbWorldPos.w == 0.0)
        return 0;

    InstanceConstants instConst = instanceConstants[gbInstId[pixelIndex]];
    Material material = materials[instConst.materialId];
    float2 matUV = gbUV[pixelIndex];
    if (material.isLight != -1)
    {
        return getBaseColor(material, matUV, mdl_textures_2d, mdl_sampler_tex);
    }
    float3 wpos = gbWPos[pixelIndex].xyz;

    float3 N = normalize(gbNormal[pixelIndex].xyz);
    float3 world_normal = N;// normalize(mul(float4(N, 0), instConst.worldToObject).xyz);

    float4 tangent0 = gbTangent[pixelIndex];
    tangent0.xyz = normalize(tangent0.xyz);
    float3 world_tangent = tangent0.xyz;//normalize(mul(instConst.objectToWorld, float4(tangent0.xyz, 0)).xyz);
    world_tangent = normalize(world_tangent - dot(world_tangent, world_normal) * world_normal);
    float3 world_binormal = cross(world_normal, world_tangent);

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    MdlMaterial currMdlMaterial = mdlMaterials[instConst.materialId];

    // setup MDL state
    Shading_state_material mdlState = (Shading_state_material) 0;
    mdlState.normal = world_normal;
    mdlState.geom_normal = world_normal;
    mdlState.position = wpos;
    mdlState.animation_time = 0.0f;
    mdlState.tangent_u[0] = world_tangent;
    mdlState.tangent_v[0] = world_binormal;
    mdlState.world_to_object = instConst.objectToWorld;
    mdlState.object_to_world = instConst.worldToObject;
    mdlState.object_id = 0;
    mdlState.meters_per_scene_unit = 1.0f;
    //fill from MDL material struct
    mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
    mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
    
    mdlState.text_coords[0] = float3(matUV, 0);

    int scatteringFunctionIndex = currMdlMaterial.functionId;
    mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    //if (ubo.debug == 1)
    {
        float3 debugN = (world_normal + 1.0) * 0.5;
        return debugN;
    }

    float3 V = normalize(wpos - ubo.camPos.xyz);

    float3 toLight;
    float lightPdf = 0;
    float3 radianceOverPdf = sampleLights(rngState, accel, mdlState, toLight, lightPdf);

    const float ior1 = 1.5f;
    const float ior2 = 1.5f;

    Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;
    evalData.ior1 = ior1;    // IOR current medium
    evalData.ior2 = ior2;    // IOR other side
    evalData.k1 = -V;        // outgoing direction
    evalData.k2 = toLight;   // incoming direction
    
    mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

    float3 finalColor = float3(0.0f);
    if (evalData.pdf > 0.0f)
    {
        const float mis_weight = lightPdf / (lightPdf + evalData.pdf + 1e-5);
        const float3 w = float3(1.0f) * radianceOverPdf * mis_weight;
        finalColor += w * evalData.bsdf_diffuse;
        finalColor += w * evalData.bsdf_glossy;
    }

    float4 rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

    Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
    sampleData.ior1 = ior1;
    sampleData.ior2 = ior2;
    sampleData.k1 = -V;
    sampleData.xi = rndSample;

    mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

    // generate new ray
    Ray ray;
    const float3 offset = world_normal * 1e-6; // need to add small offset to fix self-collision
    // add check and flip offset for transmission event
    ray.o = float4(mdlState.position + offset, 1e9);
    ray.d = float4(sampleData.k2, 0.0);

    Hit hit;
    hit.t = 0.0;
    
    float3 throughput = sampleData.bsdf_over_pdf;
    
    int depth = 1;
    int maxDepth = ubo.maxDepth;
    while (depth < maxDepth)
    {
        hit.t = 0.0;
        if (closestHit(accel, ray, hit))
        {
            instConst = accel.instanceConstants[hit.instId];
            material = materials[instConst.materialId];

            if (material.isLight != -1)
            {
                finalColor += throughput * float3(1.0f);
                //break;
                depth = maxDepth;
            }
            else
            {
                uint i0 = accel.ib[instConst.indexOffset + hit.primId * 3 + 0];
                uint i1 = accel.ib[instConst.indexOffset + hit.primId * 3 + 1];
                uint i2 = accel.ib[instConst.indexOffset + hit.primId * 3 + 2];

                float3 p0 = mul(instConst.objectToWorld, float4(accel.vb[i0].position, 1.0f)).xyz;
                float3 p1 = mul(instConst.objectToWorld, float4(accel.vb[i1].position, 1.0f)).xyz;
                float3 p2 = mul(instConst.objectToWorld, float4(accel.vb[i2].position, 1.0f)).xyz;

                float3 geom_normal = normalize(cross(p1 - p0, p2 - p0));

                float3 n0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].normal));
                float3 n1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].normal));
                float3 n2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].normal));

                float3 t0 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i0].tangent));
                float3 t1 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i1].tangent));
                float3 t2 = mul((float3x3) instConst.normalMatrix, unpackNormal(accel.vb[i2].tangent));

                const float2 bcoords = hit.bary;

                float3 world_normal = normalize(interpolateAttrib(n0, n1, n2, bcoords));
                float3 world_tangent = normalize(interpolateAttrib(t0, t1, t2, bcoords));
                float3 world_binormal = cross(world_normal, world_tangent);

                float2 uv0 = unpackUV(accel.vb[i0].uv);
                float2 uv1 = unpackUV(accel.vb[i1].uv);
                float2 uv2 = unpackUV(accel.vb[i2].uv);

                float2 uvCoord = interpolateAttrib(uv0, uv1, uv2, bcoords);

                currMdlMaterial = mdlMaterials[instConst.materialId];

                // setup MDL state
                mdlState.normal = world_normal;
                mdlState.geom_normal = geom_normal;
                mdlState.position = ray.o.xyz + ray.d.xyz * hit.t; // hit position
                mdlState.animation_time = 0.0f;
                mdlState.tangent_u[0] = world_tangent;
                mdlState.tangent_v[0] = world_binormal;
                mdlState.ro_data_segment_offset = currMdlMaterial.ro_data_segment_offset;
                mdlState.world_to_object = instConst.worldToObject;
                mdlState.object_to_world = instConst.objectToWorld; // TODO: replace on precalc
                mdlState.object_id = 0;
                mdlState.meters_per_scene_unit = 1.0f;
                mdlState.arg_block_offset = currMdlMaterial.arg_block_offset;
                mdlState.text_coords[0] = float3(uvCoord, 0);

                int scatteringFunctionIndex = currMdlMaterial.functionId;
                mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

                radianceOverPdf = sampleLights(rngState, accel, mdlState, toLight, lightPdf);

                Bsdf_evaluate_data evalData = (Bsdf_evaluate_data) 0;
                evalData.ior1 = ior1;       // IOR current medium // 1.2
                evalData.ior2 = ior2;       // IOR other side
                evalData.k1 = -ray.d.xyz; // outgoing direction
                evalData.k2 = toLight;      // incoming direction
                
                mdl_bsdf_scattering_evaluate(scatteringFunctionIndex, evalData, mdlState);

                if (evalData.pdf > 0.0f)
                {
                    const float mis_weight = lightPdf / (lightPdf + evalData.pdf + 1e-5);
                    const float3 w = throughput * radianceOverPdf * mis_weight;
                    finalColor += w * evalData.bsdf_diffuse;
                    finalColor += w * evalData.bsdf_glossy;
                }

                rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));

                Bsdf_sample_data sampleData = (Bsdf_sample_data) 0;
                sampleData.ior1 = ior1;
                sampleData.ior2 = ior2;
                sampleData.k1 = -ray.d.xyz;
                sampleData.xi = rndSample;

                mdl_bsdf_scattering_sample(scatteringFunctionIndex, sampleData, mdlState);

                throughput *= sampleData.bsdf_over_pdf;

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

                const float3 offset = world_normal * 1e-6; // need to add small offset to fix self-collision
                // add check and flip offset for transmission event
                ray.o = float4(mdlState.position + offset, 1e9);
                ray.d = float4(sampleData.k2, 0.0);
            }
        }
        else
        {
            // miss - add background color and exit
            // finalColor += throughput * float3(0.f);
            finalColor += (throughput + 1e-5) * cubeMap.Sample(cubeMapSampler, ray.d.xyz).rgb;

            //break;
            depth = maxDepth;
        }
        ++depth;
    }
    return finalColor;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 dispatchIndex : SV_DispatchThreadID)
{
    if (dispatchIndex.x >= ubo.dimension.x * ubo.dimension.y * ubo.spp)
    {
        return;
    }
    uint2 pixelIndex = uint2(dispatchIndex.x / ubo.spp % ubo.dimension.x, dispatchIndex.x / ubo.spp / ubo.dimension.x);
    uint sampleNum = dispatchIndex.x % ubo.spp;

    uint rngState = initRNG(pixelIndex, ubo.dimension, (ubo.frameNumber + 1) * (sampleNum + 1) * (ubo.iteration + 1));
    float3 color = pathTraceCameraRays(pixelIndex, rngState);
    sampleBuffer[dispatchIndex.x * 3 + 0] = color.r;
    sampleBuffer[dispatchIndex.x * 3 + 1] = color.g;
    sampleBuffer[dispatchIndex.x * 3 + 2] = color.b;
}
