#pragma once
#include <vulkan/vulkan.h>
#include <scene/scene.h>
#include <vector>
#include <array>
#include <resourcemanager.h>


namespace nevk
{
class RenderPass
{
private:
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 modelViewProj;
        alignas(16) glm::mat4 worldToView;
        alignas(16) glm::mat4 inverseWorldToView;
        alignas(4) glm::float4 lightDirect;
    };

    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
    VkRenderPass mRenderPass;
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkDevice mDevice;

    VkShaderModule mVS, mPS;

    ResourceManager* mResMngr;
    VkDescriptorPool mDescriptorPool;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkImageView mTextureImageView;
    VkSampler mTextureSampler;

    void createRenderPass();

    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);

    void createUniformBuffers();

    std::vector<VkDescriptorSet> mDescriptorSets;

    std::vector<VkFramebuffer> mFrameBuffers;

    VkFormat mFrameBufferFormat;

    VkFormat mDepthBufferFormat;
    uint32_t mWidth, mHeight;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Scene::Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

        VkVertexInputAttributeDescription attributeDescription;

        attributeDescription.binding = 0;
        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(Scene::Vertex, pos);
        attributeDescriptions.emplace_back(attributeDescription);

        attributeDescription.binding = 0;
        attributeDescription.location = 1;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(Scene::Vertex, normal);
        attributeDescriptions.emplace_back(attributeDescription);

        attributeDescription.binding = 0;
        attributeDescription.location = 2;
        attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription.offset = offsetof(Scene::Vertex, uv);
        attributeDescriptions.emplace_back(attributeDescription);

        attributeDescription.binding = 0;
        attributeDescription.location = 3;
        attributeDescription.format = VK_FORMAT_R32_UINT;
        attributeDescription.offset = offsetof(Scene::Vertex, materialId);
        attributeDescriptions.emplace_back(attributeDescription);

        return attributeDescriptions;
    }

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);

public:
    void createGraphicsPipeline(VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule, uint32_t width, uint32_t height);

    void createFrameBuffers(std::vector<VkImageView>& imageViews, VkImageView& depthImageView, uint32_t width, uint32_t height);

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
    }

    void setDepthBufferFormat(VkFormat format)
    {
        mDepthBufferFormat = format;
    }

    void setTextureImageView(VkImageView textureImageView);
    void setTextureSampler(VkSampler textureSampler);

    void init(VkDevice& device, const char* vsCode, uint32_t vsCodeSize, const char* psCode, uint32_t psCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr, uint32_t width, uint32_t height)
    {
        mDevice = device;
        mResMngr = resMngr;
        mDescriptorPool = descpool;
        mWidth = width;
        mHeight = height;
        mVS = createShaderModule(vsCode, vsCodeSize);
        mPS = createShaderModule(psCode, psCodeSize);
        createUniformBuffers();

        createRenderPass();
        createDescriptorSetLayout();
        createDescriptorSets(mDescriptorPool);
        createGraphicsPipeline(mVS, mPS, width, height);
    }

    void onResize(std::vector<VkImageView>& imageViews, VkImageView& depthImageView, uint32_t width, uint32_t height);

    void onDestroy();

    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& perspective, const glm::float4x4& view, float lightX, float lightY, float lightZ);

    RenderPass(/* args */);
    ~RenderPass();

    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);
};
} // namespace nevk
