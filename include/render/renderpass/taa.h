#pragma once

#include "renderpass/fullscreenpass.h"

namespace nevk
{

struct TaaPassInitInfo
{
    VkDevice device;
    VkDescriptorPool descriptorPool;
    std::vector<VkImageView> colorImageViews;
    uint32_t imageWidth;
    uint32_t imageHeight;
    ResourceManager* resourceManager;
    ShaderManager* shaderManager;
};

class TAA : public FullScreenPass
{
private:
    //===================================
    // Descriptor layout variables
    VkImageView mColorImageView;
    VkImageView mPrevColorImageView;
    VkSampler mSampler;

    //===================================

    void createRenderPass() override;
    void createDescriptorSetLayout() override;

public:
    TAA(/* args */);
    ~TAA();

    void updateDescriptorSets() override;
    void init(TaaPassInitInfo& info)
    {
        mShaderName = std::string("shaders/taa.hlsl");
        mDevice = info.device;
        mDescriptorPool = info.descriptorPool;
        mWidth = info.imageWidth;
        mHeight = info.imageHeight;
        mResourceManager = info.resourceManager;
        mShaderManager = info.shaderManager;

        createShaderModules();

        createDescriptorSetLayout();
        createDescriptorSets(mDescriptorPool);
        updateDescriptorSets();

        createRenderPass();
        createGraphicsPipeline();
        createFrameBuffers(info.colorImageViews);
    }

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
    }

    void setTextureImageViews(std::vector<VkImageView>& imageViews, uint32_t imageIndex = 0)
    {
        mColorImageView = imageViews[imageIndex % 2];
        mPrevColorImageView = imageViews[(imageIndex + 1) % 2];
    }

    void setTextureSampler(VkSampler imageSampler)
    {
        mSampler = imageSampler;
    }
};
} // namespace nevk
