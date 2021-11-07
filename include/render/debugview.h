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
    struct DebugImages
    {
        Image* LTC = VK_NULL_HANDLE;
        Image* shadow = VK_NULL_HANDLE;
        Image* normal = VK_NULL_HANDLE;
        Image* motion = VK_NULL_HANDLE;
        Image* debug = VK_NULL_HANDLE;
        Image* AO = VK_NULL_HANDLE;
        Image* reflection = VK_NULL_HANDLE;
        Image* variance = VK_NULL_HANDLE;
        Image* pathTracer = VK_NULL_HANDLE;
    };

    DebugView(const SharedContext& ctx);
    ~DebugView();
    void initialize();
    void setInputTexture(const DebugImages& images);
    void setOutputTexture(VkImageView imageView);
};
} // namespace nevk
