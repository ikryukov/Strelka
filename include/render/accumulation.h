#pragma once

#include "common.h"
#include "computepass.h"

#include "accumulationparam.h"

#include <vector>

namespace nevk
{
using AccumulationBase = ComputePass<AccumulationParam>;
class Accumulation : public AccumulationBase
{
public:
    Accumulation(const SharedContext& ctx);
    ~Accumulation();
    void initialize();
    void setInputTexture(Image* input);
    void setHistoryTexture(Image* history);
    void setOutputTexture(Image* output);
};
} // namespace nevk
