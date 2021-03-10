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

    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;

    VkShaderModule mCS;

    ResourceManager* mResMngr;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkImageView mTextureImageView;
    VkImageView mOutImageView;
    VkSampler mTextureSampler;

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);

    void createUniformBuffers();

    std::vector<VkDescriptorSet> mDescriptorSets;

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);

public:
    void createComputePipeline(VkShaderModule& shaderModule);

    void setTextureImageView(VkImageView textureImageView);
    void setOutputImageView(VkImageView imageView);
    void setTextureSampler(VkSampler textureSampler);

    void init(VkDevice& device, const char* csCode, uint32_t csCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr);

    void onDestroy();

    void updateUniformBuffer(uint32_t currentImage, const uint32_t width, const uint32_t height);

    ComputePass(/* args */);
    ~ComputePass();

    void record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex);
};
} // namespace nevk
