#pragma once

#include <resourcemanager/resourcemanager.h>
#include <scene/scene.h>
#include <shadermanager/ShaderManager.h>
#include <vulkan/vulkan.h>

#include <array>
#include <string>
#include <vector>

namespace nevk
{

class RenderPass
{
protected:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkRenderPass mRenderPass;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;

    std::string mShaderName;
    VkShaderModule mVertexShader, mPixelShader;

    ResourceManager* mResourceManager;
    ShaderManager* mShaderManager;

    //===================================
    // Descriptor handlers
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    //===================================
    // Framebuffer handlers
    std::vector<VkFramebuffer> mFrameBuffers;
    VkFormat mFrameBufferFormat;
    uint32_t mWidth, mHeight;

    //===================================

    virtual void createRenderPass() = 0;
    virtual void createGraphicsPipeline() = 0;
    virtual void createDescriptorSetLayout() = 0;

    void createShaderModules();
    VkShaderModule createModule(const char* code, uint32_t codeSize);
    void createDescriptorSets(VkDescriptorPool& descriptorPool);

public:
    virtual void updateDescriptorSets() = 0;
    virtual void onDestroy();

    void reloadShader()
    {
        vkDeviceWaitIdle(mDevice);
        vkDestroyPipeline(mDevice, mPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        createShaderModules();
        createGraphicsPipeline();
    }

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
    }
};

} // namespace nevk
