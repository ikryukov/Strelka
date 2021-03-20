#include "renderpass/renderpass.h"

namespace nevk
{

void RenderPass::createShaderModules()
{
    VkShaderModule oldVS = mVertexShader, oldPS = mPixelShader;
    try
    {
        uint32_t vertId = mShaderManager->loadShader(mShaderName.c_str(), "vertexMain", false);
        uint32_t fragId = mShaderManager->loadShader(mShaderName.c_str(), "fragmentMain", true);

        const char* vertShaderCode = nullptr;
        uint32_t vertShaderCodeSize = 0;
        mShaderManager->getShaderCode(vertId, vertShaderCode, vertShaderCodeSize);

        const char* fragShaderCode = nullptr;
        uint32_t fragShaderCodeSize = 0;
        mShaderManager->getShaderCode(fragId, fragShaderCode, fragShaderCodeSize);

        mVertexShader = createModule(vertShaderCode, vertShaderCodeSize);
        mPixelShader = createModule(fragShaderCode, fragShaderCodeSize);

        //if (oldVS != VK_NULL_HANDLE && oldPS != VK_NULL_HANDLE)
        //{
        //    vkDestroyShaderModule(mDevice, oldVS, nullptr);
        //    vkDestroyShaderModule(mDevice, oldPS, nullptr);
        //}
    }
    catch (std::exception& error)
    {
        mVertexShader = oldVS;
        mPixelShader = oldPS;
        std::cerr << error.what();
    }
}

VkShaderModule RenderPass::createModule(const char* code, const uint32_t codeSize)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (uint32_t*)code;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void RenderPass::createDescriptorSets(VkDescriptorPool& descriptorPool)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    mDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void RenderPass::onDestroy()
{
    for (auto frameBuffer : mFrameBuffers)
        vkDestroyFramebuffer(mDevice, frameBuffer, nullptr);
    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
    vkDestroyShaderModule(mDevice, mVertexShader, nullptr);
    vkDestroyShaderModule(mDevice, mPixelShader, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
}

} // namespace nevk
