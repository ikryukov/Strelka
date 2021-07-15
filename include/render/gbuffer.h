#pragma once
#include <vulkan/vulkan.h>
#include <resourcemanager/resourcemanager.h>

struct GBuffer
{
    nevk::Image* pos;
    VkImageView posView;
    nevk::Image* wPos;
    VkImageView wPosView;
    nevk::Image* posLightSpace;
    VkImageView posLightSpaceView;
    nevk::Image* depth;
    VkImageView depthView;
    nevk::Image* normal;
    VkImageView normalView;
    nevk::Image* tangent;
    VkImageView tangentView;
    nevk::Image* uv;
    VkImageView uvView;

    nevk::Image* instId;
    VkImageView instIdView;

    // packed
    nevk::Image* gbuffer1;
    VkImageView gbuffer1View;
    nevk::Image* gbuffer2;
    VkImageView gbuffer2View;
    nevk::Image* gbuffer3;
    VkImageView gbuffer3View;

    // utils
    VkFormat depthFormat;
    uint32_t width;
    uint32_t height;
};
