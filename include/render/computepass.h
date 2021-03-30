#pragma once
#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <resourcemanager.h>
#include <vector>

namespace nevk
{
class ComputePass
{
private:
    struct UniformBufferObject
    {
        glm::int2 dimension;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkShaderModule mCS;

    ResourceManager* mResMngr;

    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    std::vector<VkImageView> mTextureImageView;
    VkImageView mOutImageView;
    VkSampler mTextureSampler;

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void updateDescriptorSets();

    void createUniformBuffers();

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createComputePipeline(VkShaderModule& shaderModule);

public:
    ComputePass(/* args */);
    ~ComputePass();

    void init(VkDevice& device, const char* csCode, uint32_t csCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr);
    void record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex);
    void onDestroy();

    void setTextureImageView(std::vector<VkImageView> textureImageView);
    void setOutputImageView(VkImageView imageView);
    void setTextureSampler(VkSampler textureSampler);
    void updateUniformBuffer(uint32_t currentImage, const uint32_t width, const uint32_t height);
};
} // namespace nevk
