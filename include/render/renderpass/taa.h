#pragma once

#include <vulkan/vulkan.h>
#include <resourcemanager.h>
#include <shadermanager/ShaderManager.h>
#include <scene/scene.h>

#include <vector>
#include <array>
#include <string>

namespace nevk
{

class TAA
{
private:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkRenderPass mRenderPass;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;

    std::string mShaderName;
    VkShaderModule mVS, mPS;

    ResourceManager* mResManager;
    ShaderManager* mShaderManager;

    //===================================
    // Descriptor handlers
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    //===================================
    // Descriptor layout
    VkImageView mColorImageView;
    VkImageView mPrevColorImageView;
    VkSampler mSampler;

    //===================================
    // Framebuffer
    std::vector<VkFramebuffer> mFrameBuffers;
    VkFormat mFrameBufferFormat;
    uint32_t mWidth, mHeight;

    //===================================

    void createRenderPass();
    void createGraphicsPipeline(VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule, uint32_t width, uint32_t height);
    
    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void createShaderModules();
    VkShaderModule createModule(const char* code, uint32_t codeSize);

public:
    TAA(/* args */);
    ~TAA();

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
    }

    void setSampledImageView(VkImageView imageView)
    {
        mColorImageView = imageView;
    }

    void setSampledImageSampler(VkSampler imageSampler)
    {
        mSampler = imageSampler;
    }

    void reloadShader()
    {
        vkDeviceWaitIdle(mDevice);
        vkDestroyPipeline(mDevice, mPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        createShaderModules();
        createGraphicsPipeline(mVS, mPS, mWidth, mHeight);
    }

    void updateDescriptorSets();

    void record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex);
    void onResize(std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void onDestroy();

    void createFrameBuffers(std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void init(VkDevice& device, VkDescriptorPool descpool, ResourceManager* resMngr, ShaderManager* shMngr, uint32_t width, uint32_t height)
    {
        mShaderName = std::string("shaders/taa.hlsl");
        mDevice = device;
        mResManager = resMngr;
        mShaderManager = shMngr;
        mDescriptorPool = descpool;
        mWidth = width;
        mHeight = height;

        createShaderModules();
        createRenderPass();
        createDescriptorSetLayout();
        createDescriptorSets(mDescriptorPool);
        updateDescriptorSets();
        createGraphicsPipeline(mVS, mPS, width, height);
    }
};
} // namespace nevk
