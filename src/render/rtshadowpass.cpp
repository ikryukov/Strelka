#include "rtshadowpass.h"

#include <array>
#include <stdexcept>

#define GLM_FORCE_SILENT_WARNINGS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <utility>

namespace nevk
{
RtShadowPass::RtShadowPass(/* args */)
{
}

RtShadowPass::~RtShadowPass()
{
}

void RtShadowPass::createComputePipeline(VkShaderModule& shaderModule)
{
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;

    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create compute pipeline!");
    }
}

VkShaderModule RtShadowPass::createShaderModule(const char* code, const uint32_t codeSize)
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

void RtShadowPass::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.push_back(uboLayoutBinding);

    // gbuffer wpos
    VkDescriptorSetLayoutBinding texLayoutBinding{};
    texLayoutBinding.binding = 1;
    texLayoutBinding.descriptorCount = 1;
    texLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texLayoutBinding.pImmutableSamplers = nullptr;
    texLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    texLayoutBinding.binding = 1;
    bindings.push_back(texLayoutBinding);

    // gbuffer normal
    VkDescriptorSetLayoutBinding texNormalLayoutBinding{};
    texNormalLayoutBinding.binding = 1;
    texNormalLayoutBinding.descriptorCount = 1;
    texNormalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texNormalLayoutBinding.pImmutableSamplers = nullptr;
    texNormalLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    texNormalLayoutBinding.binding = 2;
    bindings.push_back(texNormalLayoutBinding);

    VkDescriptorSetLayoutBinding bvhNodeLayoutBinding{};
    bvhNodeLayoutBinding.binding = 3;
    bvhNodeLayoutBinding.descriptorCount = 1;
    bvhNodeLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bvhNodeLayoutBinding.pImmutableSamplers = nullptr;
    bvhNodeLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(bvhNodeLayoutBinding);
    
    VkDescriptorSetLayoutBinding bvhTriangleLayoutBinding{};
    bvhTriangleLayoutBinding.binding = 4;
    bvhTriangleLayoutBinding.descriptorCount = 1;
    bvhTriangleLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bvhTriangleLayoutBinding.pImmutableSamplers = nullptr;
    bvhTriangleLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(bvhTriangleLayoutBinding);
    
    VkDescriptorSetLayoutBinding lightsLayoutBinding{};
    lightsLayoutBinding.binding = 5;
    lightsLayoutBinding.descriptorCount = 1;
    lightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightsLayoutBinding.pImmutableSamplers = nullptr;
    lightsLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(lightsLayoutBinding);

    VkDescriptorSetLayoutBinding outLayoutBinding{};
    outLayoutBinding.binding = 6;
    outLayoutBinding.descriptorCount = 1;
    outLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    outLayoutBinding.pImmutableSamplers = nullptr;
    outLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(outLayoutBinding);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void RtShadowPass::createDescriptorSets(VkDescriptorPool& descriptorPool)
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

void RtShadowPass::updateDescriptorSet(uint32_t descIndex)
{
    VkDescriptorSet& dstDescSet = mDescriptorSets[descIndex];

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mResMngr->getVkBuffer(uniformBuffers[descIndex]);
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 0;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = &bufferInfo;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo imageInfoWPos{};
    imageInfoWPos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoWPos.imageView = mGbuffer ? mGbuffer->wPosView : VK_NULL_HANDLE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 1;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &imageInfoWPos;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo imageInfoNormal{};
    imageInfoNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoNormal.imageView = mGbuffer ? mGbuffer->normalView : VK_NULL_HANDLE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 2;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &imageInfoNormal;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorBufferInfo bvhNodeInfo{};
    bvhNodeInfo.buffer = mBvhNodeBuffer;
    bvhNodeInfo.offset = 0;
    bvhNodeInfo.range = VK_WHOLE_SIZE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 3;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = &bvhNodeInfo;
        descriptorWrites.push_back(descWrite);
    }
    
    VkDescriptorBufferInfo bvhTriangleInfo{};
    bvhTriangleInfo.buffer = mBvhTriangleBuffer;
    bvhTriangleInfo.offset = 0;
    bvhTriangleInfo.range = VK_WHOLE_SIZE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 4;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = &bvhTriangleInfo;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorBufferInfo lightsInfo{};
    lightsInfo.buffer = mLightsBuffer;
    lightsInfo.offset = 0;
    lightsInfo.range = VK_WHOLE_SIZE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 5;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = &lightsInfo;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo outputImageInfo{};
    outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    outputImageInfo.imageView = mOutImageView;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 6;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &outputImageInfo;
        descriptorWrites.push_back(descWrite);
    }

    vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    needDesciptorSetUpdate[descIndex] = false;
}

void RtShadowPass::updateDescriptorSets()
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        updateDescriptorSet(i);
    }
}

void RtShadowPass::record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex)
{
    if (needDesciptorSetUpdate[imageIndex])
    {
        updateDescriptorSet(imageIndex);
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mDescriptorSets[imageIndex % MAX_FRAMES_IN_FLIGHT], 0, nullptr);
    const uint32_t dispX = (width + 15) / 16;
    const uint32_t dispY = (height + 15) / 16;
    vkCmdDispatch(cmd, dispX, dispY, 1);
}

void RtShadowPass::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uniformBuffers[i] = mResMngr->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void RtShadowPass::updateUniformBuffer(uint32_t currentImage, uint32_t frameNumber, const glm::float4x4& lightSpaceMatrix, Scene& scene, uint32_t cameraIndex, const uint32_t width, const uint32_t height)
{
    UniformBufferObject ubo{};
    ubo.dimension.x = width;
    ubo.dimension.y = height;
    Camera& camera = scene.getCamera(cameraIndex);
    glm::float4x4 proj = camera.getPerspective();
    glm::float4x4 view = camera.getView();

    ubo.frameNumber = frameNumber;
    ubo.viewToProj = proj;
    ubo.CameraPos = camera.getPosition();
    ubo.worldToView = view;
    ubo.lightPosition = scene.mLightPosition;
    ubo.lightSpaceMatrix = lightSpaceMatrix;
    ubo.debugView = (uint32_t)scene.mDebugViewSettings;

    void* data = mResMngr->getMappedMemory(uniformBuffers[currentImage]);
    memcpy(data, &ubo, sizeof(ubo));
}

void RtShadowPass::onDestroy()
{
    for (size_t i = 0; i < uniformBuffers.size(); ++i)
    {
        mResMngr->destroyBuffer(uniformBuffers[i]);
    }
    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyShaderModule(mDevice, mCS, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
}

void RtShadowPass::setBvhBuffers(VkBuffer nodeBuffer, VkBuffer triangleBuffer)
{
    mBvhNodeBuffer = nodeBuffer;
    mBvhTriangleBuffer = triangleBuffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void RtShadowPass::setLightsBuffer(VkBuffer buffer)
{
    mLightsBuffer = buffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void RtShadowPass::setGbuffer(GBuffer* gbuffer)
{
    mGbuffer = gbuffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void RtShadowPass::setOutputImageView(VkImageView imageView)
{
    mOutImageView = imageView;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void RtShadowPass::init(VkDevice& device, const char* csCode, uint32_t csCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr)
{
    mDevice = device;
    mResMngr = resMngr;
    mDescriptorPool = descpool;
    mCS = createShaderModule(csCode, csCodeSize);
    createUniformBuffers();
    createDescriptorSetLayout();
    createDescriptorSets(mDescriptorPool);
    updateDescriptorSets();
    createComputePipeline(mCS);
}

} // namespace nevk
