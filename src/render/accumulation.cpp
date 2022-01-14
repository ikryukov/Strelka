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
        if (desc.input1)
        {
            param.setTexture("currTex1", mSharedCtx.mResManager->getView(desc.input1));
            param.setTexture("currTex4", VK_NULL_HANDLE);
            param.setTexture("prevTex1", mSharedCtx.mResManager->getView(desc.history1));
            param.setTexture("prevTex4", VK_NULL_HANDLE);
            param.setTexture("output1", mSharedCtx.mResManager->getView(desc.output1));
            param.setTexture("output4", VK_NULL_HANDLE);        
        }
        else if (desc.input4)
        {
            param.setTexture("currTex4", mSharedCtx.mResManager->getView(desc.input4));
            param.setTexture("currTex1", VK_NULL_HANDLE);
            param.setTexture("prevTex4", mSharedCtx.mResManager->getView(desc.history4));
            param.setTexture("prevTex1", VK_NULL_HANDLE);
            param.setTexture("output4", mSharedCtx.mResManager->getView(desc.output4));
            param.setTexture("output1", VK_NULL_HANDLE);
        }
        else
        {
            // error
            assert(0);
        }
        param.setTexture("gbWpos", mSharedCtx.mResManager->getView(desc.wpos));
        param.setTexture("motionTex", mSharedCtx.mResManager->getView(desc.motion));
        param.setTexture("prevDepthTex", mSharedCtx.mResManager->getView(desc.prevDepth));
        param.setTexture("currDepthTex", mSharedCtx.mResManager->getView(desc.currDepth));
    }
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipeline);
    VkDescriptorSet descSet = param.getDescriptorSet(frameIndex);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &descSet, 0, nullptr);
    const uint32_t dispX = (width + 15) / 16;
    const uint32_t dispY = (height + 15) / 16;
    vkCmdDispatch(cmd, dispX, dispY, 1);
}
} // namespace nevk
