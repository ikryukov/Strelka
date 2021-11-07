#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "pathtracerparam.h"

namespace nevk
{
struct PathTracerDesc
{
    GBuffer* gbuffer = VK_NULL_HANDLE;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;
    Buffer* vb = VK_NULL_HANDLE;
    Buffer* ib = VK_NULL_HANDLE;
    Image* result = VK_NULL_HANDLE;
    Buffer* materials = VK_NULL_HANDLE;
    Buffer* instanceConst = VK_NULL_HANDLE;
    std::vector<Image*> matTextures;
    std::vector<VkSampler> matSampler;
};

using PathTracerBase = ComputePass<PathTracerParam>;
class PathTracer : public PathTracerBase
{
public:
    std::vector<VkSampler> mMatSamplers;
    std::vector<Image*> mMatTextures;

    PathTracer(const SharedContext& ctx);
    ~PathTracer();

    void initialize();
    void setResources(const PathTracerDesc& desc);
};
} // namespace nevk
