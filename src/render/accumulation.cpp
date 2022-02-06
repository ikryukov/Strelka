#include "accumulation.h"

namespace nevk
{
Accumulation::Accumulation(const SharedContext& ctx)
    : AccumulationBase(ctx)
{
}
Accumulation::~Accumulation()
{
}
void Accumulation::initialize()
{
    AccumulationBase::initialize("shaders/accumulation.hlsl");
}
void Accumulation::execute(VkCommandBuffer& cmd, const AccumulationDesc& desc, uint32_t width, uint32_t height, uint64_t frameIndex)
{
    auto& param = mShaderParamFactory.getNextShaderParameters(frameIndex);
    {
        param.setConstants(desc.constants);
        {
            param.setTexture("input", mSharedCtx.mResManager->getView(desc.input));
            param.setTexture("output", mSharedCtx.mResManager->getView(desc.output));
        }
        //param.setTexture("gbWpos", mSharedCtx.mResManager->getView(desc.wpos));
        //param.setTexture("motionTex", mSharedCtx.mResManager->getView(desc.motion));
        //param.setTexture("prevDepthTex", mSharedCtx.mResManager->getView(desc.prevDepth));
        //param.setTexture("currDepthTex", mSharedCtx.mResManager->getView(desc.currDepth));
    }

    int frameVersion = frameIndex % MAX_FRAMES_IN_FLIGHT;
    NeVkResult res = updatePipeline(frameVersion);
    assert(res == NeVkResult::eOk);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, getPipeline(frameVersion));
    VkDescriptorSet descSet = param.getDescriptorSet(frameIndex);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, getPipeLineLayout(frameVersion), 0, 1, &descSet, 0, nullptr);
    const uint32_t dispX = (width + 15) / 16;
    const uint32_t dispY = (height + 15) / 16;
    vkCmdDispatch(cmd, dispX, dispY, 1);
}
} // namespace nevk
