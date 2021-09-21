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
private:
    Image* mBilateralFilter = nullptr;
    VkSampler mBilateralSampler = VK_NULL_HANDLE;

public:
    BilateralFilter(const SharedContext& ctx);
    ~BilateralFilter();
    void initialize();
    void setResources(BilateralResourceDesc& desc);
};
} // namespace nevk
