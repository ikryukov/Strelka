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

void AOPass::setResources(AOPassDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setBuffer("bvhNodes", mSharedCtx.mResManager->getVkBuffer(desc.bvhNodes));
    mShaderParams.setBuffer("bvhTriangles", mSharedCtx.mResManager->getVkBuffer(desc.bvhTriangles));
    mShaderParams.setBuffer("lights", mSharedCtx.mResManager->getVkBuffer(desc.lights));
}

} // namespace nevk
