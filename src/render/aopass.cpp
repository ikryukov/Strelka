#include "aopass.h"
namespace nevk
{

AOPass::AOPass(const SharedContext& ctx)
    : AOBase(ctx)
{
}

AOPass::~AOPass()
{
}

void AOPass::initialize()
{
    AOBase::initialize("shaders/ao.hlsl");
}

void AOPass::setResources(const AOPassDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setBuffer("bvhNodes", mSharedCtx.mResManager->getVkBuffer(desc.bvhNodes));
    mShaderParams.setBuffer("instanceConstants", mSharedCtx.mResManager->getVkBuffer(desc.instanceConstants));
    mShaderParams.setBuffer("vb", mSharedCtx.mResManager->getVkBuffer(desc.vb));
    mShaderParams.setBuffer("ib", mSharedCtx.mResManager->getVkBuffer(desc.ib));
}

} // namespace nevk
