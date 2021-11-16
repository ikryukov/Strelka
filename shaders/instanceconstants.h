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

struct InstanceConstants
{
    float4x4 objectToWorld;
    float4x4 normalMatrix;
    int32_t materialId;
    int32_t indexOffset;
    int32_t indexCount;
    int32_t pad2;
};

#ifdef __cplusplus
#    undef float4x4
#    undef float4
#    undef float3
#    undef int2
#endif
