#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
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

#ifndef __cplusplus
#include "bindless.h"

float getRoughness(Material material, float2 matUV, Texture2D textures[BINDLESS_TEXTURE_COUNT], SamplerState samplers[BINDLESS_SAMPLER_COUNT])
{
    float roughness = material.roughnessFactor;
    if (material.texMetallicRoughness != -1)
    {
        roughness *= textures[NonUniformResourceIndex(material.texMetallicRoughness)].SampleLevel(samplers[NonUniformResourceIndex(material.sampMetallicRoughness)], matUV, 0).g;
    }
    return roughness;
}

float3 getBaseColor(Material material, float2 matUV, Texture2D textures[BINDLESS_TEXTURE_COUNT], SamplerState samplers[BINDLESS_SAMPLER_COUNT])
{
    float3 dcol = material.baseColorFactor.rgb;
    if (material.texBaseColor != -1)
    {
        dcol *= textures[NonUniformResourceIndex(material.texBaseColor)].SampleLevel(samplers[NonUniformResourceIndex(material.sampBaseId)], matUV, 0).rgb;
    }
    return dcol;
}

#endif
