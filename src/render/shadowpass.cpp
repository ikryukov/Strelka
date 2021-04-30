#include "shadowpass.h"

#include <array>
#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <utility>

namespace nevk
{
ShadowPass::ShadowPass(/* args */)
{
}

ShadowPass::~ShadowPass()
{
}
//Rendering the shadow map w/o color attachments
void ShadowPass::createShadowPass()
{

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = NULL;
    subpass.pResolveAttachments = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{}; /* if we need it here */
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = { depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(mDevice, &renderPassInfo, NULL, &mShadowPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

//shadow pass pipeline w/o fragment shader
void ShadowPass::createGraphicsPipeline(VkShaderModule& shadowShaderModule)
{
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfo.module = shadowShaderModule;
    shaderStageInfo.pName = "main";

    VkVertexInputBindingDescription bindingDescription{};
    VkVertexInputAttributeDescription attributeDescription{};

// here binding position
    bindingDescription.binding = 0;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDescription.stride = sizeof(Scene::Vertex);

    attributeDescription.binding = 0;
    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = offsetof(Scene::Vertex, pos);

    VkPipelineVertexInputStateCreateInfo vertexPipeline;
    vertexPipeline.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexPipeline.pNext = NULL;
    vertexPipeline.flags = 0;
    vertexPipeline.vertexBindingDescriptionCount = 1;
    vertexPipeline.pVertexBindingDescriptions = &bindingDescription;
    vertexPipeline.vertexAttributeDescriptionCount = 1; // only pos
    vertexPipeline.pVertexAttributeDescriptions = &attributeDescription;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;

    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.pVertexInputState = &vertexPipeline;
    pipelineInfo.pStages = &shaderStageInfo;
    pipelineInfo.stageCount = 1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create compute pipeline!");
    }
}

VkShaderModule ShadowPass::createShaderModule(const char* code, const uint32_t codeSize)
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

void ShadowPass::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // for depth image ?
   /*  VkDescriptorSetLayoutBinding texLayoutBinding{};
    texLayoutBinding.binding = 1;
    texLayoutBinding.descriptorCount = 1;
    texLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texLayoutBinding.pImmutableSamplers = nullptr;
    texLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; */


    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void ShadowPass::createDescriptorSets(VkDescriptorPool& descriptorPool)
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

/* update desc sets ? */

void ShadowPass::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        mResMngr->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
    }
}

glm::float4x4 ShadowPass::computeMVP()
{
    glm::vec3 lightInvDir = glm::vec3(0.5f,2,2);

    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
    glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 depthModelMatrix = glm::mat4(1.0);

    return depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
}

void ShadowPass::updateUniformBuffer(uint32_t currentImage, const glm::float4x4& perspective, const glm::float4x4& view, const glm::float4& lightPosition, const glm::float3& camPos)
{
    UniformBufferObject ubo{};
    glm::float4x4 model = glm::float4x4(1.0f);
    glm::float4x4 proj = perspective;

    ubo.MVP = computeMVP();
    ubo.modelToWorld = model;
    ubo.modelViewProj = proj * view * model;
    //ubo.lightPosition = lightPosition;  ?
    ubo.lightPosition = glm::float4(camPos, 1.0f);

    void* data;
    vkMapMemory(mDevice, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(mDevice, uniformBuffersMemory[currentImage]);
}

void ShadowPass::onDestroy()
{
    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyShaderModule(mDevice, mSS, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
}

void ShadowPass::setInImageView(VkImageView textureImageView)
{
    mInImageView = textureImageView; // depth image?
}

void ShadowPass::record(VkCommandBuffer& cmd, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mShadowPass;
    //renderPassInfo.framebuffer = mFrameBuffers[imageIndex % MAX_FRAMES_IN_FLIGHT];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { width, height };

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = (float)height;
    viewport.width = (float)width;
    viewport.height = -(float)height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[imageIndex % MAX_FRAMES_IN_FLIGHT], 0, nullptr);

    vkCmdDrawIndexed(cmd, indicesCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd);
}

void ShadowPass::init(VkDevice& device, const char* ssCode, uint32_t ssCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr)
{
    mDevice = device;
    mResMngr = resMngr;
    mDescriptorPool = descpool;
    mSS = createShaderModule(ssCode, ssCodeSize);
    createUniformBuffers();
    createDescriptorSetLayout();
    createDescriptorSets(mDescriptorPool);
    createGraphicsPipeline(mSS);
}

} // namespace nevk
