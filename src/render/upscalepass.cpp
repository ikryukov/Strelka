#include "upscalepass.h"

namespace nevk
{
UpscalePass::UpscalePass(const SharedContext& ctx)
    : UpscalePassBase(ctx)
{

}
UpscalePass::~UpscalePass()
{
    vkDestroySampler(mSharedCtx.mDevice, mUpscaleSampler, nullptr);
}
void UpscalePass::initialize()
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

    VkResult res = vkCreateSampler(mSharedCtx.mDevice, &samplerInfo, nullptr, &mUpscaleSampler);
    if (res != VK_SUCCESS)
    {
        // error
        assert(0);
    }

    mShaderParams.setSampler("upscaleSampler", mUpscaleSampler);

    UpscalePassBase::initialize("shaders/upscalepass.hlsl");
}
void UpscalePass::setInputTexture(VkImageView input)
{
    mShaderParams.setTexture("input", input);
}
void UpscalePass::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
