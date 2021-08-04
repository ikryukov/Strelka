#pragma once
#include <scene/scene.h>
#include <vulkan/vulkan.h>

#include <resourcemanager.h>
#include <vector>

#include "gbuffer.h"

namespace nevk
{
class RtShadowPass
{
private:
    struct UniformBufferObject
    {
        glm::float4x4 viewToProj;
        glm::float4x4 worldToView;
        glm::float4x4 lightSpaceMatrix;
        glm::float4 lightPosition;
        glm::float3 CameraPos;
        float pad0;
        glm::int2 dimension;
        uint32_t debugView;
        float pad1;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkDescriptorPool mDescriptorPool;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkShaderModule mCS;

    ResourceManager* mResMngr = nullptr;

    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    bool needDesciptorSetUpdate;
    int imageViewCounter = 0;
    
    std::vector<Buffer*> uniformBuffers;

    GBuffer* mGbuffer = nullptr;
    VkBuffer mBvhBuffer = VK_NULL_HANDLE;
    VkBuffer mLightsBuffer = VK_NULL_HANDLE;

    VkImageView mOutImageView = VK_NULL_HANDLE;

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void updateDescriptorSet(uint32_t descIndex);
    void updateDescriptorSets();

    void createUniformBuffers();

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createComputePipeline(VkShaderModule& shaderModule);

public:
    RtShadowPass(/* args */);
    ~RtShadowPass();

    void init(VkDevice& device, const char* csCode, uint32_t csCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr);
    void record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex);
    void onDestroy();

    void setBvhBuffer(VkBuffer buffer);
    void setLightsBuffer(VkBuffer buffer);
    void setGbuffer(GBuffer* gbuffer);
    void setOutputImageView(VkImageView imageView);
    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& lightSpaceMatrix, Scene& scene, uint32_t cameraIndex, const uint32_t width, const uint32_t height);
};
} // namespace nevk
