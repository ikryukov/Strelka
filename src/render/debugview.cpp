#include "debugview.h"

namespace nevk
{
DebugView::DebugView(const SharedContext& ctx)
    : DebugViewBase(ctx)
{
}
DebugView::~DebugView()
{
}
void DebugView::initialize()
{
    DebugViewBase::initialize("shaders/debugview.hlsl");
}
void DebugView::execute(VkCommandBuffer& cmd, const DebugDesc& desc, uint32_t width, uint32_t height, uint64_t frameIndex)
{
    auto& param = mShaderParamFactory.getNextShaderParameters(frameIndex);
    {
        param.setConstants(desc.constants);
        
        param.setTexture("inputNormals", mSharedCtx.mResManager->getView(desc.normal));
        param.setTexture("inputMotion", mSharedCtx.mResManager->getView(desc.motion));
        param.setTexture("debugTex", mSharedCtx.mResManager->getView(desc.debug));
        param.setTexture("inputPathTracer", mSharedCtx.mResManager->getView(desc.pathTracer));
        param.setTexture("output", mSharedCtx.mResManager->getView(desc.output));
    }
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipeline);
    VkDescriptorSet descSet = param.getDescriptorSet(frameIndex);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout, 0, 1, &descSet, 0, nullptr);
    const uint32_t dispX = (width + 15) / 16;
    const uint32_t dispY = (height + 15) / 16;
    vkCmdDispatch(cmd, dispX, dispY, 1);
}
} // namespace nevk
