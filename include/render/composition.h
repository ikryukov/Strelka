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
    struct CompositionImages
    {
        Image* LTC = VK_NULL_HANDLE;
        Image* shadow = VK_NULL_HANDLE;
        Image* AO = VK_NULL_HANDLE;
        Image* reflections = VK_NULL_HANDLE;
        Image* pathTracer = VK_NULL_HANDLE;

    };
    void initialize();
    void setInputTexture(const CompositionImages& images);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
