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
    void setInputTexture1(Image* input);
    void setInputTexture4(Image* input);
    void setWposTexture(Image* input);
    void setMotionTexture(Image* motion);
    void setPrevDepthTexture(Image* input);
    void setCurrDepthTexture(Image* input);
    void setHistoryTexture1(Image* history);
    void setOutputTexture1(Image* output);
    void setHistoryTexture4(Image* history);
    void setOutputTexture4(Image* output);
};
} // namespace nevk
