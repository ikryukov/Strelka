#include "reduction.h"

namespace nevk
{
ReductionPass::ReductionPass(const SharedContext& ctx)
    : ReductionPassBase(ctx)
{
}
ReductionPass::~ReductionPass()
{
}
void ReductionPass::initialize()
{
    ReductionPassBase::initialize("shaders/reduction.hlsl");
}
void ReductionPass::execute(VkCommandBuffer& cmd, const ReductionDesc& desc, uint32_t width, uint32_t height, uint64_t frameIndex)
{
    assert(height);
    auto& param = mShaderParamFactory.getNextShaderParameters(frameIndex);
    {
        param.setConstants(desc.constants);

        param.setTexture("output", mSharedCtx.mResManager->getView(desc.result));
        param.setBuffer("sampleBuffer", mSharedCtx.mResManager->getVkBuffer(desc.sampleBuffer));
    }
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipeline);
    VkDescriptorSet descSet = param.getDescriptorSet(frameIndex);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &descSet, 0, nullptr);
    const uint32_t dispX = (width + 255) / 256;
    const uint32_t dispY = 1;
    vkCmdDispatch(cmd, dispX, dispY, 1);
}
} // namespace nevk
