#pragma once
#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <resourcemanager.h>
#include <vector>

namespace nevk
{
class ShadowPass
{
private:
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 MVP;
        alignas(16) glm::mat4 modelToWorld;
        alignas(16) glm::mat4 modelViewProj;
        alignas(16) glm::float4 lightPosition;
    };

    const uint32_t SHADOW_MAP_WIDTH = 1024;
    const uint32_t SHADOW_MAP_HEIGHT = 1024;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkShaderModule mSS;

    ResourceManager* mResMngr;

    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkImageView mInImageView;

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void updateDescriptorSets(); //?

    void createUniformBuffers();

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createGraphicsPipeline(VkShaderModule& shadowShaderModule);
    VkFormat findDepthFormat();

    static glm::float4x4 computeMVP();

public:
    ShadowPass(/* args */);
    ~ShadowPass();

    VkRenderPass mShadowPass;

    void createShadowPass();
    void init(VkDevice& device, const char* ssCode, uint32_t ssCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr);
    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex); //?
    void onDestroy();

    void setInImageView(VkImageView textureImageView);
    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& perspective, const glm::float4x4& view, const glm::float4& lightPosition, const glm::float3& camPos);
};
} // namespace nevk
