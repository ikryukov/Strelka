#pragma once

#include "common.h"
#include "computepass.h"

#include "compositionparam.h"

#include <vector>

namespace nevk
{
using CompositionBase = ComputePass<Compositionparam>;
class Composition : public CompositionBase
{
public:
    Composition(const SharedContext& ctx);
    ~Composition();
    void initialize();
    void setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadows, VkImageView imageViewAO);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
