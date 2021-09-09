#pragma once
#include "gbuffer.h"

#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <resourcemanager.h>
#include <texturemanager.h>
#include <vector>

namespace nevk
{
class LtcPass
{
private:
    struct UniformBufferObject
    {
        glm::float4x4 viewToProj;
        glm::float4x4 worldToView;
        glm::float3 CameraPos;
        uint32_t frameNumber;
        glm::int2 dimension;
        float pad0;
        float pad1;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkShaderModule mCS;

    ResourceManager* mResManager = nullptr;

    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    bool needDesciptorSetUpdate[MAX_FRAMES_IN_FLIGHT] = { false, false, false };

    std::vector<Buffer*> uniformBuffers;

    GBuffer* mGbuffer = nullptr;
    VkBuffer mLightsBuffer = VK_NULL_HANDLE;
    VkBuffer mMaterialBuffer = VK_NULL_HANDLE;
    VkBuffer mInstanceConstantsBuffer = VK_NULL_HANDLE;

    VkImageView mLtc1ImageView = VK_NULL_HANDLE;
    nevk::Image* mLtc1Image = nullptr;
    VkImageView mLtc2ImageView = VK_NULL_HANDLE;
    nevk::Image* mLtc2Image = nullptr;
    VkSampler mLtcSampler = VK_NULL_HANDLE;

    std::vector<VkImageView> mTextureImageView;
    std::vector<VkSampler> mTextureSamplers;

    VkImageView mOutImageView = VK_NULL_HANDLE;

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void updateDescriptorSet(uint32_t descIndex);
    void updateDescriptorSets();

    void createUniformBuffers();

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createComputePipeline(VkShaderModule& shaderModule);

public:
    LtcPass(/* args */);
    ~LtcPass();

    void init(VkDevice& device, const char* csCode, uint32_t csCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr, TextureManager& texMngr);
    void record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex);
    void onDestroy();

    void setLightsBuffer(VkBuffer buffer);
    void setMaterialsBuffer(VkBuffer buffer);
    void setInstanceBuffer(VkBuffer buffer);
    void setTextureSamplers(std::vector<VkSampler>& textureSamplers);
    void setTextureImageViews(const std::vector<VkImageView>& texImages);
    void setLtcResources(VkImageView ltc1, VkImageView ltc2, VkSampler ltcSampler);
    void setGbuffer(GBuffer* gbuffer);
    void setOutputImageView(VkImageView imageView);
    void updateUniformBuffer(uint32_t currentImage, uint64_t frameNumber, Scene& scene, uint32_t cameraIndex, const uint32_t width, const uint32_t height);
};
} // namespace nevk