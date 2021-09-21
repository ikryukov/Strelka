#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "bilateralparam.h"

namespace nevk
{
struct BilateralResourceDesc
{
    GBuffer* gbuffer;
    Image* input;
    Image* result;
};

using BilateralFilterBase = ComputePass<BilateralParam>;
class BilateralFilter : public BilateralFilterBase
{
public:
    BilateralFilter(const SharedContext& ctx);
    ~BilateralFilter();
    void initialize();
    void setResources(BilateralResourceDesc& desc);
};
} // namespace nevk
