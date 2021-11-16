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
#    define int2 glm::int2
#    define uint glm::uint
#endif

struct PathTracerParam
{
    int2 dimension;
    uint frameNumber;
    uint maxDepth;

    float4 camPos;

    uint debug;
    uint numLights;
    int pad0;
    int pad1;
};

#ifdef __cplusplus
#    undef float4x4
#    undef float4
#    undef float3
#    undef int2
#endif
