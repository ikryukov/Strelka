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
void ReductionPass::setResources(const ReductionDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));
    mShaderParams.setBuffer("sampleBuffer", mSharedCtx.mResManager->getVkBuffer(desc.sampleBuffer));
    // mShaderParams.setBuffer("compositingBuffer", mSharedCtx.mResManager->getVkBuffer(desc.compositingBuffer));
}
} // namespace nevk
