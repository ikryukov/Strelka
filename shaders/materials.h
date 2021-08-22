#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#endif

struct Material
{
    float4 diffuse; // Kd
    float4 baseColorFactor;
    uint32_t illum; // illum 2 -- модель освещения //4 bytes
    int32_t texNormalId = -1; // map_normal - map_Bump
    float d;
    float metallicFactor;

    float roughnessFactor;
    int32_t texBaseColor = -1;
    int32_t texEmissive = -1;
    int32_t sampEmissiveId = -1;

    int32_t texOcclusion = -1;
    int32_t sampOcclusionId = -1;
    int32_t sampBaseId = -1;
    int32_t sampNormalId = -1;

    float3 emissiveFactor;
    int32_t pad0;

    bool isTransparent()
    {
        // TODO:
        return illum != 2;
    }
};
