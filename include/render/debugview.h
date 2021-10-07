#pragma once

#include "common.h"
#include "computepass.h"
#include "debugviewparam.h"

#include <vector>

namespace nevk
{
using DebugViewBase = ComputePass<Debugviewparam>;
class DebugView : public DebugViewBase
{
public:
    struct DebugImageViews
    {
        VkImageView imageViewLTC;
        VkImageView imageViewShadow;
        VkImageView imageNormal;
        VkImageView imageMotion;
        VkImageView debug;
        VkImageView imageAO;
    };

    DebugView(const SharedContext& ctx);
    ~DebugView();
    void initialize();
    void setInputTexture(DebugImageViews imageViews);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
