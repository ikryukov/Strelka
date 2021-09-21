#include "bilateralfilter.h"

namespace nevk
{

BilateralFilter::BilateralFilter(const SharedContext& ctx)
    : BilateralFilterBase(ctx)
{
}

BilateralFilter::~BilateralFilter()
{
    mSharedCtx.mResManager->destroyImage(mBilateralFilter);
    vkDestroySampler(mSharedCtx.mDevice, mBilateralSampler, nullptr);
}

void BilateralFilter::initialize()
{
    BilateralFilterBase::initialize("shaders/bilateralfilter.hlsl");
}

void BilateralFilter::setResources(BilateralResourceDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));
    mShaderParams.setTexture("input", mSharedCtx.mResManager->getView(desc.input));
    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setTexture("depth", mSharedCtx.mResManager->getView(desc.gbuffer->depth));
}

} // namespace nevk
