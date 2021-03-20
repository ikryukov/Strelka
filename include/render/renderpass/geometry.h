#pragma once

#include "renderpass/renderpass.h"

namespace nevk
{

struct GeometryPassInitInfo
{
    VkDevice device;
    VkDescriptorPool descriptorPool;
    VkImageView colorImageView;
    VkImageView depthImageView;
    uint32_t imageWidth;
    uint32_t imageHeight;
    ResourceManager* resourceManager;
    ShaderManager* shaderManager;
};

class GeometryPass : public RenderPass
{
private:
    //===================================
    // Descriptor handlers
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    //===================================
    // Descriptor layout variables
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 modelViewProj;
        alignas(16) glm::mat4 worldToView;
        alignas(16) glm::mat4 inverseWorldToView;
    };
    VkImageView mTextureImageView;
    VkSampler mTextureSampler;

    //===================================
    // Framebuffer handlers
    VkFormat mDepthBufferFormat;

    //===================================

    void createRenderPass() override;
    void createGraphicsPipeline() override;
    void createDescriptorSetLayout() override;

    void createFrameBuffer(VkImageView& imageView, VkImageView& depthImageView);
    void createUniformBuffers();

public:
    GeometryPass(/* args */);
    ~GeometryPass();

    void updateDescriptorSets() override;

    void updateUniformBuffer(uint32_t imageIndex, const glm::float4x4& perspective, const glm::float4x4& view);
    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);
    void onResize(VkImageView& imageView, VkImageView& depthImageView, uint32_t width, uint32_t height);
    void init(GeometryPassInitInfo& info)
    {
        mShaderName = std::string("shaders/geometry.hlsl");
        mDevice = info.device;
        mDescriptorPool = info.descriptorPool;
        mWidth = info.imageWidth;
        mHeight = info.imageHeight;
        mResourceManager = info.resourceManager;
        mShaderManager = info.shaderManager;

        createShaderModules();
        createUniformBuffers();

        createDescriptorSetLayout();
        createDescriptorSets(mDescriptorPool);
        updateDescriptorSets();

        createRenderPass();
        createGraphicsPipeline();
        createFrameBuffer(info.colorImageView, info.depthImageView);
    }

    void setDepthBufferFormat(VkFormat format)
    {
        mDepthBufferFormat = format;
    }

    void setTextureImageView(VkImageView textureImageView)
    {
        mTextureImageView = textureImageView;
    }

    void setTextureSampler(VkSampler textureSampler)
    {
        mTextureSampler = textureSampler;
    }

private:
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
};
} // namespace nevk
