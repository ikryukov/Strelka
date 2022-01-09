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

struct RectLight
{
    float4 points[16];
    float4 color;
    float4 normal;
    int type;
    float pad0;
    float pad2;
    float pad3;
};

struct UniformLight
{
    int type; // 0 -- rect, 1 -- disc, 2 -- sphere, 3 -- cylinder
    float pad0;
    float pad1;
    float pad2;
    float4 params[4];
    // rectangle light: type == 1; params[0] -- v0, params[1] -- v1, params[2] -- v3, params[3][0] -- height, params[3][1] -- width
    // disc light: type == 1; params[0] -- v0, params[1] -- v1, params[2] -- v3, params[3][0] -- radius
    // sphere light: type == 1; params[0] -- v0, params[1] -- v1, params[2] -- v3, params[3][0] -- radius
};
