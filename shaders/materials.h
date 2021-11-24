#pragma once

#ifdef __cplusplus
#    define GLM_FORCE_SILENT_WARNINGS
#    define GLM_LANG_STL11_FORCED
#    define GLM_ENABLE_EXPERIMENTAL
#    define GLM_FORCE_CTOR_INIT
#    define GLM_FORCE_RADIANS
#    define GLM_FORCE_DEPTH_ZERO_TO_ONE
#    include <glm/glm.hpp>
#    include <glm/gtx/compatibility.hpp>
#    define float4 glm::float4
#    define float3 glm::float3
#    define uint glm::uint
#endif

struct Material
{
    float4 diffuse; // Kd
    float4 specular;
    float4 baseColorFactor;

    uint32_t illum; // illum 2 -- модель освещения
    int32_t texNormalId = -1; // map_normal - map_Bump
    float d;
    int32_t texMetallicRoughness = -1;

    int32_t sampMetallicRoughness = -1;
    float metallicFactor;
    float roughnessFactor;
    int32_t texBaseColor = -1;

    int32_t texEmissive = -1;
    int32_t sampEmissiveId = -1;
    int32_t texOcclusion = -1;
    int32_t sampOcclusionId = -1;

    float3 emissiveFactor;
    int32_t sampBaseId = -1;

    int32_t sampNormalId = -1;
    int32_t isLight = 0;
    float extIOR = 1.0f;
    float intIOR = 1.5f;

    bool isTransparent()
    {
        // TODO:
        return illum != 2;
    }
};

struct MdlMaterial
{
    int arg_block_offset = 0;
    int ro_data_segment_offset = 0;
    int functionId = 0;
    int pad1;
};

/// Information passed to GPU for mapping id requested in the runtime functions to buffer
/// views of the corresponding type.
struct Mdl_resource_info
{
    // index into the tex2d, tex3d, ... buffers, depending on the type requested
    uint gpu_resource_array_start;
    uint pad0;
    uint pad1;
    uint pad2;
};

#ifndef __cplusplus
#include "bindless.h"

float getRoughness(Material material, float2 matUV, Texture2D textures[BINDLESS_TEXTURE_COUNT], SamplerState sampler)
{
    float roughness = material.roughnessFactor;
    if (material.texMetallicRoughness != -1)
    {
        roughness *= textures[NonUniformResourceIndex(material.texMetallicRoughness)].SampleLevel(sampler, matUV, 0).g;
    }
    return roughness;
}

float3 getBaseColor(Material material, float2 matUV, Texture2D textures[BINDLESS_TEXTURE_COUNT], SamplerState sampler)
{
    float3 dcol = material.baseColorFactor.rgb;
    if (material.texBaseColor != -1)
    {
        dcol *= textures[NonUniformResourceIndex(material.texBaseColor)].SampleLevel(sampler, matUV, 0).rgb;
    }
    return dcol;
}

#endif

#ifdef __cplusplus
#    undef float4
#    undef float3
#    undef uint
#endif