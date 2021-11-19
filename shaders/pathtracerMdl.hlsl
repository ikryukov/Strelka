#include "helper.h"
#include "materials.h"
#include "pack.h"
#include "lights.h"
#include "helper.h"
#include "raytracing.h"
#include "pathtracerparam.h"
#include "instanceconstants.h"

ConstantBuffer<PathTracerParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> gbTangent;
Texture2D<int> gbInstId;
Texture2D<float2> gbUV;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<Vertex> vb;
StructuredBuffer<uint> ib;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;
StructuredBuffer<RectLight> lights;

RWTexture2D<float4> output;

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
    float3 lightNormal = calcLightNormal(light);
    float3 Li = light.color.rgb;

    if (dot(state.normal, L) > 0 && -dot(L, lightNormal) > 0.0 && all(Li))
    {
        float distToLight = distance(pointOnLight, state.position);
        float lightArea = calcLightArea(light);
        float lightPDF = distToLight * distToLight / (-dot(L, lightNormal) * lightArea);

        Ray shadowRay;
        shadowRay.d = float4(L, 0.0);
        const float3 offset = state.normal * 1e-6; // need to add small offset to fix self-collision
        shadowRay.o = float4(state.position + offset, distToLight - 1e-5);

        Hit shadowHit;
        shadowHit.t = 0.0;
        float visibility = anyHit(accel, shadowRay, shadowHit) ? 0.0f : 1.0f;

        toLight = L;
        lightPdf = lightPDF;
        return visibility * Li * saturate(dot(state.normal, L)) / lightPDF;
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
    return r / lightPdf;
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


    // setup MDL state
    Shading_state_material mdlState = (Shading_state_material) 0;

    mdlState.normal = world_normal;
    mdlState.geom_normal = world_normal;
    mdlState.position = wpos;
    mdlState.animation_time = 0.0f;
    mdlState.tangent_u[0] = world_tangent;
    mdlState.tangent_v[0] = world_binormal;

    mdlState.ro_data_segment_offset = 0;
    mdlState.world_to_object = instConst.objectToWorld;
    mdlState.object_to_world = instConst.worldToObject; // TODO: replace on precalc
    mdlState.object_id = 0;
    mdlState.meters_per_scene_unit = 1.0f;
    mdlState.arg_block_offset = 0;
    
    mdlState.text_coords[0] = float3(matUV, 0);

    int scatteringFunctionIndex = 0;
    mdl_bsdf_scattering_init(scatteringFunctionIndex, mdlState);

    Accel accel;
    accel.bvhNodes = bvhNodes;
    accel.instanceConstants = instanceConstants;
    accel.vb = vb;
    accel.ib = ib;

    if (ubo.debug == 1)
    {
        float3 debugN = (world_normal + 1.0) * 0.5;
        return debugN;
    }

    float4 rndSample = float4(rand(rngState), rand(rngState), rand(rngState), rand(rngState));
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
        //finalColor = evalData.bsdf_glossy;
        //finalColor = float3(dot(N, toLight));
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
