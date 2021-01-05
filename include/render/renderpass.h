#pragma once
#include <vulkan/vulkan.h>
#include <ShaderManager.h>

namespace nevk
{
class RenderPass
{
private:
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayoutInfo;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDevice mDevice;
    nevk::ShaderManager& mShaderManager;
    void createRenderPass();
    void createDescriptorSetLayout();

    VkFormat mFrameBufferFormat;
    VkFormat mDepthBufferFormat;

public:
    void createGraphicsPipeline(const char* vsPath, const char* psPath);

    void setFrameBufferFormat(VkFormat format)
    {
        mFrameBufferFormat = format;
    }

    void setDepthBufferFormat(VkFormat format)
    {
        mDepthBufferFormat = format;
    }

    void init()
    {
        createRenderPass();
        createDescriptorSetLayout();
    }

    RenderPass(/* args */);
    ~RenderPass();

    void record(VkCommandBuffer& cmd, VkFramebuffer& framebuffer, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);
};
} // namespace nevk
