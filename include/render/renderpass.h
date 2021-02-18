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

//    static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions()
//    {
//        std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};
//
//        attributeDescriptions[0].binding = 0;
//        attributeDescriptions[0].location = 0;
//        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[0].offset = offsetof(Scene::Vertex, pos);
//
//        attributeDescriptions[1].binding = 0;
//        attributeDescriptions[1].location = 1;
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = offsetof(Scene::Vertex, ka);
//
//        attributeDescriptions[2].binding = 0;
//        attributeDescriptions[2].location = 2;
//        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[2].offset = offsetof(Scene::Vertex, kd);
//
//        attributeDescriptions[3].binding = 0;
//        attributeDescriptions[3].location = 3;
//        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[3].offset = offsetof(Scene::Vertex, ks);
//
//        attributeDescriptions[4].binding = 0;
//        attributeDescriptions[4].location = 4;
//        attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
//        attributeDescriptions[4].offset = offsetof(Scene::Vertex, uv);
//
//        attributeDescriptions[5].binding = 0;
//        attributeDescriptions[5].location = 5;
//        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[5].offset = offsetof(Scene::Vertex, color);
//
//        attributeDescriptions[6].binding = 0;
//        attributeDescriptions[6].location = 6;
//        attributeDescriptions[6].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[6].offset = offsetof(Scene::Vertex, normal);
//
//        return attributeDescriptions;
//    }
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

        VkVertexInputAttributeDescription attributeDescription;

        attributeDescription.binding = 0;
        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(Scene::Vertex, pos);
        attributeDescriptions.emplace_back(attributeDescription);

//        attributeDescription.binding = 0;
//        attributeDescription.location = 1;
//        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescription.offset = offsetof(Scene::Vertex, ka);
//        attributeDescriptions.emplace_back(attributeDescription);
//
//        attributeDescription.binding = 0;
//        attributeDescription.location = 2;
//        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescription.offset = offsetof(Scene::Vertex, kd);
//        attributeDescriptions.emplace_back(attributeDescription);
//
//        attributeDescription.binding = 0;
//        attributeDescription.location = 3;
//        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescription.offset = offsetof(Scene::Vertex, ks);
//        attributeDescriptions.emplace_back(attributeDescription);

        attributeDescription.binding = 0;
        attributeDescription.location = 1;
        attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription.offset = offsetof(Scene::Vertex, uv);
        attributeDescriptions.emplace_back(attributeDescription);

//        attributeDescription.binding = 0;
//        attributeDescription.location = 5;
//        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescription.offset = offsetof(Scene::Vertex, color);
//        attributeDescriptions.emplace_back(attributeDescription);

        attributeDescription.binding = 0;
        attributeDescription.location = 2;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(Scene::Vertex, normal);
        attributeDescriptions.emplace_back(attributeDescription);

        attributeDescription.binding = 0;
        attributeDescription.location = 3;
        attributeDescription.format = VK_FORMAT_R32G32B32_UINT;              // check !
        attributeDescription.offset = offsetof(Scene::Vertex, materialId);
        attributeDescriptions.emplace_back(attributeDescription);

        return attributeDescriptions;
    }

    VkShaderModule createShaderModule(const char* code, const uint32_t codeSize);

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

    void updateUniformBuffer(uint32_t currentImage);

    RenderPass(/* args */);
    ~RenderPass();

    void record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);
};
} // namespace nevk
