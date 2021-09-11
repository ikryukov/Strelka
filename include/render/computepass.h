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

    VkShaderModule createShaderModule(const char* code, uint32_t codeSize);
    void createComputePipeline(VkShaderModule& shaderModule);

public:
    ComputePass(const SharedContext& ctx);
    ~ComputePass();

    void initialize();
    void execute(VkCommandBuffer& cmd, uint32_t width, uint32_t height, uint32_t imageIndex);
    void onDestroy();

};
} // namespace nevk
