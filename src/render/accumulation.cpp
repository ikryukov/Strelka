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
void Accumulation::setInputTexture1(Image* input)
{
    mShaderParams.setTexture("currTex1", mSharedCtx.mResManager->getView(input));
    mShaderParams.setTexture("currTex4", VK_NULL_HANDLE);
}
void Accumulation::setInputTexture4(Image* input)
{
    mShaderParams.setTexture("currTex4", mSharedCtx.mResManager->getView(input));
    mShaderParams.setTexture("currTex1", VK_NULL_HANDLE);
}
void Accumulation::setWposTexture(Image* input)
{
    mShaderParams.setTexture("gbWpos", mSharedCtx.mResManager->getView(input));
}
void Accumulation::setMotionTexture(Image* motion)
{
    mShaderParams.setTexture("motionTex", mSharedCtx.mResManager->getView(motion));
}
void Accumulation::setPrevDepthTexture(Image* input)
{
    mShaderParams.setTexture("prevDepthTex", mSharedCtx.mResManager->getView(input));
}
void Accumulation::setCurrDepthTexture(Image* input)
{
    mShaderParams.setTexture("currDepthTex", mSharedCtx.mResManager->getView(input));
}
void Accumulation::setHistoryTexture1(Image* history)
{
    mShaderParams.setTexture("prevTex1", mSharedCtx.mResManager->getView(history));
    mShaderParams.setTexture("prevTex4", VK_NULL_HANDLE);
}
void Accumulation::setHistoryTexture4(Image* history)
{
    mShaderParams.setTexture("prevTex4", mSharedCtx.mResManager->getView(history));
    mShaderParams.setTexture("prevTex1", VK_NULL_HANDLE);
}
void Accumulation::setOutputTexture1(Image* output)
{
    mShaderParams.setTexture("output1", mSharedCtx.mResManager->getView(output));
    mShaderParams.setTexture("output4", VK_NULL_HANDLE);
}
void Accumulation::setOutputTexture4(Image* output)
{
    mShaderParams.setTexture("output4", mSharedCtx.mResManager->getView(output));
    mShaderParams.setTexture("output1", VK_NULL_HANDLE);
}
} // namespace nevk
