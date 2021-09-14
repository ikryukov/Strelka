#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "rtshadowparam.h"

namespace nevk
{
struct RtShadowPassDesc
{
    GBuffer* gbuffer = nullptr;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* bvhTriangles = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;

    Image* result = VK_NULL_HANDLE;
};

using RtShadowBase = ComputePass<RtShadowParam>;
class RtShadowPass : public RtShadowBase
{
public:
    RtShadowPass(const SharedContext& ctx);
    ~RtShadowPass();

    void initialize();
    void setResources(RtShadowPassDesc& desc);
};
} // namespace nevk
