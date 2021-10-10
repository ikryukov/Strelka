#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "reflectionparam.h"

namespace nevk
{
struct ReflectionDesc
{
    GBuffer* gbuffer = nullptr;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* bvhTriangles = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;
    Image* result = VK_NULL_HANDLE;
    Buffer* materials;
    Buffer* instanceConst;
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
