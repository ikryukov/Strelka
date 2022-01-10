#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#endif

struct Light
{
    float3 v0;
    float pad0;
    float3 v1;
    float pad1;
    float3 v2;
    float pad2;
};

struct UniformLight
{
    float4 points[16];
    float4 color;
    float4 normal;
    int type;
    float pad0;
    float pad2;
    float pad3;
};
