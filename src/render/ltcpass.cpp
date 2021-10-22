#include "ltcpass.h"

// matrices
#include "ltc_data.h"

namespace nevk
{

LtcPass::LtcPass(const SharedContext& ctx)
    : LtcPassBase(ctx)
{
}

LtcPass::~LtcPass()
{
    mSharedCtx.mResManager->destroyImage(mLtc1);
    mSharedCtx.mResManager->destroyImage(mLtc2);
    vkDestroySampler(mSharedCtx.mDevice, mLtcSampler, nullptr);
}

void LtcPass::initialize()
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

    VkResult res = vkCreateSampler(mSharedCtx.mDevice, &samplerInfo, nullptr, &mLtcSampler);
    if (res != VK_SUCCESS)
    {
        // error
        assert(0);
    }

    nevk::TextureManager::Texture ltc1Tex = mSharedCtx.mTextureManager->createTextureImage(g_ltc_1, 4 * sizeof(float), VK_FORMAT_R32G32B32A32_SFLOAT, 64, 64);
    nevk::TextureManager::Texture ltc2Tex = mSharedCtx.mTextureManager->createTextureImage(g_ltc_2, 4 * sizeof(float), VK_FORMAT_R32G32B32A32_SFLOAT, 64, 64);

    mLtc1 = ltc1Tex.textureImage;
    mLtc2 = ltc2Tex.textureImage;

    mShaderParams.setTexture("ltc1", mSharedCtx.mResManager->getView(mLtc1));
    mShaderParams.setTexture("ltc2", mSharedCtx.mResManager->getView(mLtc2));
    
    mShaderParams.setSampler("ltcSampler", mLtcSampler);

    LtcPassBase::initialize("shaders/ltc.hlsl");
}

void LtcPass::setResources(const LtcResourceDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setTexture("gbUV", mSharedCtx.mResManager->getView(desc.gbuffer->uv));
    mShaderParams.setTexture("gbInstId", mSharedCtx.mResManager->getView(desc.gbuffer->instId));
    mShaderParams.setBuffer("instanceConstants", mSharedCtx.mResManager->getVkBuffer(desc.instanceConst));
    mShaderParams.setBuffer("lights", mSharedCtx.mResManager->getVkBuffer(desc.lights));
    mShaderParams.setBuffer("materials", mSharedCtx.mResManager->getVkBuffer(desc.materials));

    mMatSamplers = desc.matSampler;
    mMatTextures = desc.matTextures;
    mShaderParams.setSamplers("samplers", mMatSamplers);
    mShaderParams.setTextures("textures", mMatTextures);
}

} // namespace nevk
