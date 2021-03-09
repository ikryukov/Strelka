#pragma once

#include <vulkan/vulkan.h>
#include <resourcemanager.h>
#include <shadermanager/ShaderManager.h>
#include <scene/scene.h>

#include <vector>
#include <array>
#include <string>

namespace nevk
{

class GeometryPass
{
private:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;

    VkDevice mDevice;
    VkRenderPass mRenderPass;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;

    std::string mShaderName;
    VkShaderModule mVS, mPS;

    ResourceManager* mResManager;
    ShaderManager* mShaderManager;

    //===================================
    // Descriptor handlers
    VkDescriptorPool mDescriptorPool;
    VkDescriptorSetLayout mDescriptorSetLayout;
    std::vector<VkDescriptorSet> mDescriptorSets;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    //===================================
    // Descriptor layouts
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 modelViewProj;
        alignas(16) glm::mat4 worldToView;
        alignas(16) glm::mat4 inverseWorldToView;
    };
    VkImageView mTextureImageView;
    VkSampler mTextureSampler;

    //===================================
    // Framebuffer
    VkFramebuffer mFrameBuffer;
    VkFormat mFrameBufferFormat;
    VkFormat mDepthBufferFormat;
    uint32_t mWidth, mHeight;

    //===================================

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

    void createRenderPass();
    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void createUniformBuffers();
    void createShaderModules();
    void createGraphicsPipeline(VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule, uint32_t width, uint32_t height);

    VkShaderModule createModule(const char* code, uint32_t codeSize);

public:
    GeometryPass(/* args */);
    ~GeometryPass();

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
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

    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage, const glm::float4x4& perspective, const glm::float4x4& view);
    void onResize(VkImageView& imageView, VkImageView& depthImageView, uint32_t width, uint32_t height);
    void onDestroy();

    void createFrameBuffer(VkImageView& imageView, VkImageView& depthImageView, uint32_t width, uint32_t height);
    void init(VkDevice& device, VkDescriptorPool descpool, ResourceManager* resMngr, ShaderManager* shMngr, uint32_t width, uint32_t height)
    {
        mShaderName = std::string("shaders/geometry.hlsl");
        mDevice = device;
        mResManager = resMngr;
        mShaderManager = shMngr;
        mDescriptorPool = descpool;
        mWidth = width;
        mHeight = height;

        createShaderModules();
        createUniformBuffers();

        createRenderPass();
        createDescriptorSetLayout();
        createDescriptorSets(mDescriptorPool);
        createGraphicsPipeline(mVS, mPS, width, height);
    }
};
} // namespace nevk
