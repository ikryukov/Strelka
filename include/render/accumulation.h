#pragma once

#include "common.h"
#include "computepass.h"

#include "accumulationparam.h"

namespace nevk
{
struct AccumulationDesc
{
    AccumulationParam constants;

    Image* wpos = VK_NULL_HANDLE;
    Image* motion = VK_NULL_HANDLE;
    Image* prevDepth = VK_NULL_HANDLE;
    Image* currDepth = VK_NULL_HANDLE;

    Image* history1 = VK_NULL_HANDLE;
    Image* history4 = VK_NULL_HANDLE;

    Image* input1 = VK_NULL_HANDLE;
    Image* input4 = VK_NULL_HANDLE;

    Image* output1 = VK_NULL_HANDLE;
    Image* output4 = VK_NULL_HANDLE;
};

using AccumulationBase = ComputePass<AccumulationParam>;
class Accumulation : public AccumulationBase
{
public:
    Accumulation(const SharedContext& ctx);
    ~Accumulation();
    void initialize();
    
    void execute(VkCommandBuffer& cmd, const AccumulationDesc& desc, uint32_t width, uint32_t height, uint64_t frameIndex);
};
} // namespace nevk
