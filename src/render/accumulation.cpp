#include "accumulation.h"

namespace nevk
{
Accumulation::Accumulation(const SharedContext& ctx)
    : AccumulationBase(ctx)
{
}
Accumulation::~Accumulation()
{
}
void Accumulation::initialize()
{
    AccumulationBase::initialize("shaders/accumulation.hlsl");
}
void Accumulation::setInputTexture(Image* input)
{
    mShaderParams.setTexture("currTex", mSharedCtx.mResManager->getView(input));
}
void Accumulation::setHistoryTexture(Image* history)
{
    mShaderParams.setTexture("prevTex", mSharedCtx.mResManager->getView(history));
}
void Accumulation::setOutputTexture(Image* output)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(output));
}
} // namespace nevk
