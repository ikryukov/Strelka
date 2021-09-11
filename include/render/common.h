#pragma once
#include <vulkan/vulkan.h>

#include <ShaderManager.h>
#include <resourcemanager.h>

namespace nevk
{
static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

#ifdef __APPLE__
const uint32_t BINDLESS_TEXTURE_COUNT = 64;
const uint32_t BINDLESS_SAMPLER_COUNT = 15;
#else
const uint32_t BINDLESS_TEXTURE_COUNT = 2048;
const uint32_t BINDLESS_SAMPLER_COUNT = 36;
#endif

struct SharedContext
{
    VkDevice mDevice = VK_NULL_HANDLE;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    ResourceManager* mResManager = nullptr;
    ShaderManager* mShaderManager = nullptr;
};

enum class NeVkResult: uint32_t
{
    eOk,
    eFail,
    eOutOfMemory,
};

} // namespace nevk
