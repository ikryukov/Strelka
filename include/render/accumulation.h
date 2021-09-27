#pragma once

#include "common.h"
#include "computepass.h"

#include "accumulationparam.h"

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
    void setWposTexture(Image* input);
    void setMotionTexture(Image* motion);
    void setPrevDepthTexture(Image* input);
    void setCurrDepthTexture(Image* input);
    void setHistoryTexture(Image* history);
    void setOutputTexture(Image* output);
};
} // namespace nevk
