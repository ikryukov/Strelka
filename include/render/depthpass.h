#pragma once
#include "debugUtils.h"

#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <resourcemanager.h>
#include <vector>

namespace nevk
{
class DepthPass
{
private:
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 lightSpaceMatrix;
        alignas(16) glm::mat4 modelToWorld;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkFramebuffer shadowMapFb;

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

    bool mEnableValidation = false;

    void beginLabel(VkCommandBuffer cmdBuffer, const char* labelName, const glm::float4& color)
    {
        if (mEnableValidation)
        {
            nevk::debug::beginLabel(cmdBuffer, labelName, color);
        }
    }

    void endLabel(VkCommandBuffer cmdBuffer)
    {
        if (mEnableValidation)
        {
            nevk::debug::endLabel(cmdBuffer);
        }
    }

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void updateDescriptorSets(); //?

    void createUniformBuffers();

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createGraphicsPipeline(VkShaderModule& shadowShaderModule, uint32_t width, uint32_t height);

    static glm::mat4 computeLightSpaceMatrix();
    bool needDesciptorSetUpdate;
    int imageviewcounter = 0;

    void updateDescriptorSets(uint32_t descSetIndex);

public:
    DepthPass(/* args */);
    ~DepthPass();

    VkRenderPass mShadowPass;

    void createShadowPass();
    void init(VkDevice& device, bool enableValidation, const char* ssCode, uint32_t ssCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr, uint32_t width, uint32_t height);
    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, nevk::Scene& scene, uint32_t width, uint32_t height, uint32_t imageIndex); //?
    void createFrameBuffers(VkImageView& shadowImageView, uint32_t width, uint32_t height);
    void onDestroy();

    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& perspective, const glm::float4x4& view, const glm::float4& lightPosition, const glm::float3& camPos);
};
} // namespace nevk
