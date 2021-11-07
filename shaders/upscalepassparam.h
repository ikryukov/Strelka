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
#    define float2 glm::float2
#    define int2 glm::int2
#endif

struct Upscalepassparam
{
    int2 dimension;
    float2 invDimension;
};

#ifdef __cplusplus
#    undef float4
#    undef float3
#    undef float2
#    undef int2
#endif
