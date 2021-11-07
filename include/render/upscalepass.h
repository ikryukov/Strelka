#pragma once

#include "common.h"
#include "computepass.h"

#include "upscalepassparam.h"

#include <vector>

namespace nevk
{
using UpscalePassBase = ComputePass<Upscalepassparam>;
class UpscalePass : public UpscalePassBase
{
private:
    VkSampler mUpscaleSampler = VK_NULL_HANDLE;
public:
    UpscalePass(const SharedContext& ctx);
    ~UpscalePass();
    void initialize();
    void setInputTexture(VkImageView input);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
