#pragma once

#include <resourcemanager/resourcemanager.h>
#include <shadermanager/ShaderManager.h>
#include <texturemanager/texturemanager.h>
#include <vulkan/vulkan.h>

#include <settings/settings.h>

namespace oka
{
static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

struct FrameData
{
    VkCommandBuffer cmdBuffer;
    VkCommandPool cmdPool;
};

struct FrameSyncData
{
    VkFence inFlightFence;
    VkFence imagesInFlight;
    VkSemaphore renderFinished;
    VkSemaphore imageAvailable;
};

struct SharedContext
{
    VkDevice mDevice = VK_NULL_HANDLE;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    ResourceManager* mResManager = nullptr;
    ShaderManager* mShaderManager = nullptr;
    TextureManager* mTextureManager = nullptr;

    FrameData mFramesData[MAX_FRAMES_IN_FLIGHT] = {};
    uint64_t mFrameNumber = 0;
    uint32_t mFrameIndex = 0; // swapchain image index
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    SettingsManager* mSettingsManager = nullptr;

    FrameData& getCurrentFrameData()
    {
        return mFramesData[mFrameNumber % MAX_FRAMES_IN_FLIGHT];
    }
    FrameData& getFrameData(uint32_t idx)
    {
        return mFramesData[idx % MAX_FRAMES_IN_FLIGHT];
    }
};

enum class Result : uint32_t
{
    eOk,
    eFail,
    eOutOfMemory,
};

} // namespace oka
