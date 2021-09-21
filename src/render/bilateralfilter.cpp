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
    // sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult res = vkCreateSampler(mSharedCtx.mDevice, &samplerInfo, nullptr, &mBilateralSampler);
    if (res != VK_SUCCESS)
    {
        // error
        assert(0);
    }

    nevk::TextureManager::Texture bilateralTex{}; //?
    mBilateralFilter = bilateralTex.textureImage;
   // mShaderParams.setTexture("bilateralTexture", mSharedCtx.mResManager->getView(mBilateralFilter));
    mShaderParams.setSampler("bilateralSampler", mBilateralSampler);

    BilateralFilterBase::initialize("shaders/bilateralfilter.hlsl");
}


void BilateralFilter::setResources(BilateralResourceDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setTexture("gbUV", mSharedCtx.mResManager->getView(desc.gbuffer->uv));
    mShaderParams.setTexture("gbInstId", mSharedCtx.mResManager->getView(desc.gbuffer->instId));
    mShaderParams.setBuffer("instanceConstants", mSharedCtx.mResManager->getVkBuffer(desc.instanceConst));
    mShaderParams.setBuffer("lights", mSharedCtx.mResManager->getVkBuffer(desc.lights));
    mShaderParams.setBuffer("materials", mSharedCtx.mResManager->getVkBuffer(desc.materials));

    mShaderParams.setSamplers("samplers", desc.matSampler);
    mShaderParams.setTextures("textures", desc.matTextures);
}

} // namespace nevk
