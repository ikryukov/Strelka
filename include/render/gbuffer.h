#pragma once
#include <resourcemanager/resourcemanager.h>
#include <vulkan/vulkan.h>

namespace nevk
{

struct GBuffer
{
    Image* wPos;
    Image* depth;
    Image* normal;
    Image* tangent;
    Image* uv;
    Image* instId;

    // utils
    VkFormat depthFormat;
    uint32_t width;
    uint32_t height;
};
} // namespace nevk
