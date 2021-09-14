#include "rtshadowpass.h"
namespace nevk
{

RtShadowPass::RtShadowPass(const SharedContext& ctx)
    : RtShadowBase(ctx)
{
}

RtShadowPass::~RtShadowPass()
{
}

void RtShadowPass::initialize()
{
    RtShadowBase::initialize("shaders/rtshadows.hlsl");
}

void RtShadowPass::setResources(RtShadowPassDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setBuffer("bvhNodes", mSharedCtx.mResManager->getVkBuffer(desc.bvhNodes));
    mShaderParams.setBuffer("bvhTriangles", mSharedCtx.mResManager->getVkBuffer(desc.bvhTriangles));
    mShaderParams.setBuffer("lights", mSharedCtx.mResManager->getVkBuffer(desc.lights));
}

} // namespace nevk
