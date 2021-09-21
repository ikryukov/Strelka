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
    DebugView(const SharedContext& ctx);
    ~DebugView();
    void initialize();
    void setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadow, VkImageView imageNormal);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
