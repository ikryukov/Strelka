#pragma once

#include "common.h"
#include "computepass.h"
#include "pathtracerparam.h"

namespace nevk
{

struct ReductionDesc
{
    Buffer* sampleBuffer;
    // Buffer* compositingBuffer;
    Image* result;
};

using ReductionPassBase = ComputePass<PathTracerParam>;
class ReductionPass : public ReductionPassBase
{
public:
    ReductionPass(const SharedContext& ctx);
    ~ReductionPass();
    void initialize();
    void setResources(const ReductionDesc& desc);
};
} // namespace nevk
