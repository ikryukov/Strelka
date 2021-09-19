#pragma once
#include <resourcemanager/resourcemanager.h>
#include <vulkan/vulkan.h>

namespace nevk
{

struct GBuffer
{
    ResourceManager* mResManager = nullptr;
    Image* wPos;
    Image* depth;
    Image* normal;
    Image* tangent;
    Image* uv;
    Image* instId;
    Image* motion;

    // utils
    VkFormat depthFormat;
    uint32_t width;
    uint32_t height;

    ~GBuffer()
    {
        assert(mResManager);
        if (wPos)
        {
            mResManager->destroyImage(wPos);
        }
        if (depth)
        {
            mResManager->destroyImage(depth);
        }
        if (normal)
        {
            mResManager->destroyImage(normal);
        }
        if (tangent)
        {
            mResManager->destroyImage(tangent);
        }
        if (uv)
        {
            mResManager->destroyImage(uv);
        }
        if (instId)
        {
            mResManager->destroyImage(instId);
        }
        if (motion)
        {
            mResManager->destroyImage(motion);
        }
    }
};
} // namespace nevk
