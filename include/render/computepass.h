#pragma once

#include "common.h"
#include "shaderparameters.h"

#include <array>

namespace nevk
{
template <typename T>
class ComputePass
{
protected:
    const SharedContext& mSharedCtx;

    VkPipeline mPipeline = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    VkShaderModule mCS = VK_NULL_HANDLE;

    ShaderParameters<T> mShaderParams;

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = codeSize;
        createInfo.pCode = (uint32_t*)code;

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(mSharedCtx.mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
    void createComputePipeline(VkShaderModule& shaderModule)
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo{};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = "main";

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        VkDescriptorSetLayout layout = mShaderParams.getDescriptorSetLayout();
        pipelineLayoutInfo.pSetLayouts = &layout;

        if (vkCreatePipelineLayout(mSharedCtx.mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = shaderStageInfo;
        pipelineInfo.layout = mPipelineLayout;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateComputePipelines(mSharedCtx.mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute pipeline!");
        }
    }

public:
    ComputePass(const SharedContext& ctx)
        : mSharedCtx(ctx)
    {
    }
    virtual ~ComputePass()
    {
    }

    void initialize(const char* shaderFile)
    {
        const char* csShaderCode = nullptr;
        uint32_t csShaderCodeSize = 0;
        uint32_t csId = mSharedCtx.mShaderManager->loadShader(shaderFile, "computeMain", nevk::ShaderManager::Stage::eCompute);
        mSharedCtx.mShaderManager->getShaderCode(csId, csShaderCode, csShaderCodeSize);
        mCS = createShaderModule(csShaderCode, csShaderCodeSize);

        mShaderParams.create(mSharedCtx, csId);

        createComputePipeline(mCS);
    }
    void execute(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &mShaderParams.getDescriptorSet(imageIndex), 0, nullptr);
        const uint32_t dispX = (width + 15) / 16;
        const uint32_t dispY = (height + 15) / 16;
        vkCmdDispatch(cmd, dispX, dispY, 1);
    }
    void onDestroy()
    {
        vkDestroyPipeline(mSharedCtx.mDevice, mPipeline, nullptr);
        vkDestroyPipelineLayout(mSharedCtx.mDevice, mPipelineLayout, nullptr);
        vkDestroyShaderModule(mSharedCtx.mDevice, mCS, nullptr);
    }

};
} // namespace nevk
