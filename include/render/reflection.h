#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "reflectionparam.h"

namespace nevk
{
struct ReflectionDesc
{
    GBuffer* gbuffer = VK_NULL_HANDLE;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* vb = VK_NULL_HANDLE;
    Buffer* ib = VK_NULL_HANDLE;
    Image* result = VK_NULL_HANDLE;
    Buffer* materials = VK_NULL_HANDLE;
    Buffer* instanceConst = VK_NULL_HANDLE;
    std::vector<Image*> matTextures;
    std::vector<VkSampler> matSampler;
};

using ReflectionBase = ComputePass<ReflectionParam>;
class Reflection : public ReflectionBase
{
public:
    Reflection(const SharedContext& ctx);
    ~Reflection();

    void initialize();
    void setResources(ReflectionDesc& desc);
};
} // namespace nevk
