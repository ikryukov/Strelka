#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "pathtracerparam.h"

namespace nevk
{
struct PathTracerDesc
{
    GBuffer* gbuffer = nullptr;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;

    Buffer* instanceConstants = VK_NULL_HANDLE;
    Buffer* vb = VK_NULL_HANDLE;
    Buffer* ib = VK_NULL_HANDLE;

    Image* result = VK_NULL_HANDLE;
};

using PathTracerBase = ComputePass<PathTracerParam>;
class PathTracer : public PathTracerBase
{
public:
    PathTracer(const SharedContext& ctx);
    ~PathTracer();

    void initialize();
    void setResources(const PathTracerDesc& desc);
};
} // namespace nevk
