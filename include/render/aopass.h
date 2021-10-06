#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "aopassparam.h"

namespace nevk
{
struct AOPassDesc
{
    GBuffer* gbuffer = nullptr;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* bvhTriangles = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;

    Image* result = VK_NULL_HANDLE;
};

using AOBase = ComputePass<AOParam>;
class AOPass : public AOBase
{
public:
    AOPass(const SharedContext& ctx);
    ~AOPass();

    void initialize();
    void setResources(AOPassDesc& desc);
};
} // namespace nevk
