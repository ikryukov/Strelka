#pragma once

#include "common.h"
#include "computepass.h"

#include "tonemapparam.h"

#include <vector>

namespace nevk
{
using TonemapBase = ComputePass<Tonemapparam>;
class Tonemap : public TonemapBase
{
public:
    Tonemap(const SharedContext& ctx);
    ~Tonemap();
    void initialize();
    void setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadows);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
