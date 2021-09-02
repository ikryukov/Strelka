#include "ltcpass.h"

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
LtcPass::LtcPass(/* args */)
{
}

LtcPass::~LtcPass()
{
}

void LtcPass::createComputePipeline(VkShaderModule& shaderModule)
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

VkShaderModule LtcPass::createShaderModule(const char* code, const uint32_t codeSize)
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

void LtcPass::createDescriptorSetLayout()
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
    bindings.push_back(texLayoutBinding);

    // gbuffer normal
    VkDescriptorSetLayoutBinding texNormalLayoutBinding{};
    texNormalLayoutBinding.binding = 2;
    texNormalLayoutBinding.descriptorCount = 1;
    texNormalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texNormalLayoutBinding.pImmutableSamplers = nullptr;
    texNormalLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(texNormalLayoutBinding);

    // gbuffer inst id
    VkDescriptorSetLayoutBinding texInstIdLayoutBinding{};
    texInstIdLayoutBinding.binding = 3;
    texInstIdLayoutBinding.descriptorCount = 1;
    texInstIdLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texInstIdLayoutBinding.pImmutableSamplers = nullptr;
    texInstIdLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(texInstIdLayoutBinding);

    VkDescriptorSetLayoutBinding instConstLayoutBinding{};
    instConstLayoutBinding.binding = 4;
    instConstLayoutBinding.descriptorCount = 1;
    instConstLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instConstLayoutBinding.pImmutableSamplers = nullptr;
    instConstLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(instConstLayoutBinding);

    VkDescriptorSetLayoutBinding lightsLayoutBinding{};
    lightsLayoutBinding.binding = 5;
    lightsLayoutBinding.descriptorCount = 1;
    lightsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightsLayoutBinding.pImmutableSamplers = nullptr;
    lightsLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(lightsLayoutBinding);

    VkDescriptorSetLayoutBinding materialsLayoutBinding{};
    materialsLayoutBinding.binding = 6;
    materialsLayoutBinding.descriptorCount = 1;
    materialsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    materialsLayoutBinding.pImmutableSamplers = nullptr;
    materialsLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(materialsLayoutBinding);

    VkDescriptorSetLayoutBinding ltc1LayoutBinding{};
    ltc1LayoutBinding.binding = 7;
    ltc1LayoutBinding.descriptorCount = 1;
    ltc1LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    ltc1LayoutBinding.pImmutableSamplers = nullptr;
    ltc1LayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(ltc1LayoutBinding);

    VkDescriptorSetLayoutBinding ltc2LayoutBinding{};
    ltc2LayoutBinding.binding = 8;
    ltc2LayoutBinding.descriptorCount = 1;
    ltc2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    ltc2LayoutBinding.pImmutableSamplers = nullptr;
    ltc2LayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(ltc2LayoutBinding);

    VkDescriptorSetLayoutBinding ltc2SamplerLayoutBinding{};
    ltc2SamplerLayoutBinding.binding = 9;
    ltc2SamplerLayoutBinding.descriptorCount = 1;
    ltc2SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    ltc2SamplerLayoutBinding.pImmutableSamplers = nullptr;
    ltc2SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings.push_back(ltc2SamplerLayoutBinding);

    VkDescriptorSetLayoutBinding outLayoutBinding{};
    outLayoutBinding.binding = 10;
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

void LtcPass::createDescriptorSets(VkDescriptorPool& descriptorPool)
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

void LtcPass::updateDescriptorSet(uint32_t descIndex)
{
    VkDescriptorSet& dstDescSet = mDescriptorSets[descIndex];

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mResManager->getVkBuffer(uniformBuffers[descIndex]);
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

    VkDescriptorImageInfo imageInfoInstId{};
    imageInfoInstId.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoInstId.imageView = mGbuffer ? mGbuffer->instIdView : VK_NULL_HANDLE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 3;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &imageInfoInstId;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorBufferInfo instConstInfo{};
    instConstInfo.buffer = mInstanceConstantsBuffer;
    instConstInfo.offset = 0;
    instConstInfo.range = VK_WHOLE_SIZE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 4;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = &instConstInfo;
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

    VkDescriptorBufferInfo materialsInfo{};
    materialsInfo.buffer = mMaterialBuffer;
    materialsInfo.offset = 0;
    materialsInfo.range = VK_WHOLE_SIZE;

    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 6;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descWrite.descriptorCount = 1;
        descWrite.pBufferInfo = &materialsInfo;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo imageInfoLtc1{};
    imageInfoLtc1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoLtc1.imageView = mLtc1ImageView;
    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 7;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &imageInfoLtc1;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo imageInfoLtc2{};
    imageInfoLtc2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoLtc2.imageView = mLtc2ImageView;
    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 8;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &imageInfoLtc2;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo imageInfoLtcSampler{};
    imageInfoLtcSampler.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfoLtcSampler.sampler = mLTCSampler;
    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 9;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &imageInfoLtcSampler;
        descriptorWrites.push_back(descWrite);
    }

    VkDescriptorImageInfo outputImageInfo{};
    outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    outputImageInfo.imageView = mOutImageView;
    {
        VkWriteDescriptorSet descWrite{};
        descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrite.dstSet = dstDescSet;
        descWrite.dstBinding = 10;
        descWrite.dstArrayElement = 0;
        descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descWrite.descriptorCount = 1;
        descWrite.pImageInfo = &outputImageInfo;
        descriptorWrites.push_back(descWrite);
    }

    vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    needDesciptorSetUpdate[descIndex] = false;
}

void LtcPass::updateDescriptorSets()
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        updateDescriptorSet(i);
    }
}

void LtcPass::record(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex)
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

void LtcPass::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        uniformBuffers[i] = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void LtcPass::updateUniformBuffer(uint32_t currentImage, uint64_t frameNumber, Scene& scene, uint32_t cameraIndex, const uint32_t width, const uint32_t height)
{
    UniformBufferObject ubo{};
    ubo.dimension.x = width;
    ubo.dimension.y = height;
    Camera& camera = scene.getCamera(cameraIndex);
    glm::float4x4 proj = camera.getPerspective();
    glm::float4x4 view = camera.getView();

    ubo.frameNumber = (uint32_t)frameNumber; // it uses as seed for random number
    ubo.viewToProj = proj;
    ubo.CameraPos = camera.getPosition();
    ubo.worldToView = view;

    void* data = mResManager->getMappedMemory(uniformBuffers[currentImage]);
    memcpy(data, &ubo, sizeof(ubo));
}

void LtcPass::onDestroy()
{
    for (size_t i = 0; i < uniformBuffers.size(); ++i)
    {
        mResManager->destroyBuffer(uniformBuffers[i]);
    }
    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyShaderModule(mDevice, mCS, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
}

void LtcPass::setLightsBuffer(VkBuffer buffer)
{
    mLightsBuffer = buffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void LtcPass::setMaterialsBuffer(VkBuffer buffer)
{
    mMaterialBuffer = buffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void LtcPass::setInstanceBuffer(VkBuffer buffer)
{
    mInstanceConstantsBuffer = buffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void LtcPass::setLtcResources(VkImageView ltc1, VkImageView ltc2, VkSampler ltcSampler)
{
    mLtc1ImageView = ltc1;
    mLtc2ImageView = ltc2;
    mLTCSampler = ltcSampler;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void LtcPass::setGbuffer(GBuffer* gbuffer)
{
    mGbuffer = gbuffer;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void LtcPass::setOutputImageView(VkImageView imageView)
{
    mOutImageView = imageView;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        needDesciptorSetUpdate[i] = true;
    }
}

void LtcPass::init(VkDevice& device, const char* csCode, uint32_t csCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr)
{
    mDevice = device;
    mResManager = resMngr;
    mDescriptorPool = descpool;
    mCS = createShaderModule(csCode, csCodeSize);
    createUniformBuffers();


    // sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult res = vkCreateSampler(mDevice, &samplerInfo, nullptr, &mLTCSampler);
    if (res != VK_SUCCESS)
    {
        // error
        assert(0);
    }

    mLtc1Image = mResManager->createImage(64, 64, VK_FORMAT_R16G16B16A16_SFLOAT,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Ltc1");
    mLtc1ImageView = mResManager->createImageView(mLtc1Image, VK_IMAGE_ASPECT_COLOR_BIT);
    mLtc2Image = mResManager->createImage(64, 64, VK_FORMAT_R16G16B16A16_SFLOAT,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Ltc2");
    mLtc2ImageView = mResManager->createImageView(mLtc2Image, VK_IMAGE_ASPECT_COLOR_BIT);

    createDescriptorSetLayout();
    createDescriptorSets(mDescriptorPool);
    updateDescriptorSets();
    createComputePipeline(mCS);
}

} // namespace nevk
