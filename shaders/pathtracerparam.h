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
#    define float4x4 glm::float4x4
#    define float3 glm::float3
#    define float2 glm::float2
#    define int2 glm::int2
#    define uint glm::uint
#endif

struct PathTracerParam
{
    float4x4 viewToWorld;
    float4x4 clipToView;
    float4x4 viewToClip;
    float4x4 worldToView;

    float4 camPos;

    int2 dimension;
    float2 invDimension;

    uint frameNumber;
    uint maxDepth;
    uint debug;
    uint numLights;

    int len;
    int pad0;
    int2 pad1;
};

#ifdef __cplusplus
#    undef float4x4
#    undef float4
#    undef float2
#    undef float3
#    undef int2
#endif
