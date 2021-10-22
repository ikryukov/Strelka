#pragma once
#include <vulkan/vulkan.h>

#include <ShaderManager.h>
#include <resourcemanager.h>
#include <texturemanager.h>
#include "bindless.h"

namespace nevk
{
static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

struct SharedContext
{
    VkDevice mDevice = VK_NULL_HANDLE;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    ResourceManager* mResManager = nullptr;
    ShaderManager* mShaderManager = nullptr;
    TextureManager* mTextureManager = nullptr;
};

enum class NeVkResult: uint32_t
{
    eOk,
    eFail,
    eOutOfMemory,
};

} // namespace nevk
