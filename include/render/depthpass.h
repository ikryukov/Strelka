#pragma once
#include "debugUtils.h"
#include "resourcemanager/resourcemanager.h"

#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <vector>

namespace nevk
{
class DepthPass
{
private:
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 lightSpaceMatrix;
    };

    struct InstancePushConstants
    {
        int32_t instanceId = -1;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkFramebuffer shadowMapFb;

    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkShaderModule mSS;
    VkBuffer mInstanceBuffer = VK_NULL_HANDLE;

    ResourceManager* mResManager;

    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    std::vector<Buffer*> uniformBuffers;

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
    void updateDescriptorSets(uint32_t descSetIndex);

    void createConstantBuffers();

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createGraphicsPipeline(VkShaderModule& shadowShaderModule, uint32_t width, uint32_t height);

    bool needDesciptorSetUpdate[MAX_FRAMES_IN_FLIGHT] = {false, false, false};
    int imageviewcounter = 0;

public:
    DepthPass(/* args */);
    ~DepthPass();

    VkRenderPass mShadowPass;

    glm::mat4 computeLightSpaceMatrix(glm::float3& lightPosition);
    glm::vec3 lightUpwards = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 lightAt = glm::vec3(0.0f);
    float depthBiasConstant = 1.25f; // Constant depth bias factor (always applied)
    float depthBiasSlope = 2.0f; // Slope depth bias factor, applied depending on polygon's slope
    float fovAngle = 45.0f;
    float zNear = 0.01f;
    float zFar = 50.f;

    void createShadowPass();
    void init(VkDevice& device, bool enableValidation, const char* ssCode, uint32_t ssCodeSize, VkDescriptorPool descpool,
        ResourceManager* resMngr, uint32_t width, uint32_t height);
    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, nevk::Scene& scene, 
        uint32_t width, uint32_t height, uint32_t imageIndex, uint32_t cameraIndex);
    void createFrameBuffers(VkImageView& shadowImageView, uint32_t width, uint32_t height);
    void onDestroy();

    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& lightSpaceMatrix);
    
    void setInstanceBuffer(VkBuffer instanceBuffer);
};
} // namespace nevk
