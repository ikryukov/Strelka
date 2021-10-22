#include "debugview.h"

namespace nevk
{
DebugView::DebugView(const SharedContext& ctx)
    : DebugViewBase(ctx)
{
}
DebugView::~DebugView()
{
}
void DebugView::initialize()
{
    DebugViewBase::initialize("shaders/debugview.hlsl");
}
void DebugView::setInputTexture(const DebugImages& images)
{
    mShaderParams.setTexture("inputLTC", mSharedCtx.mResManager->getView(images.LTC));
    mShaderParams.setTexture("inputShadow", mSharedCtx.mResManager->getView(images.shadow));
    mShaderParams.setTexture("inputNormals", mSharedCtx.mResManager->getView(images.normal));
    mShaderParams.setTexture("inputMotion", mSharedCtx.mResManager->getView(images.motion));
    mShaderParams.setTexture("debugTex", mSharedCtx.mResManager->getView(images.debug));
    mShaderParams.setTexture("inputAO", mSharedCtx.mResManager->getView(images.AO));
    mShaderParams.setTexture("inputReflection", mSharedCtx.mResManager->getView(images.reflection));
    mShaderParams.setTexture("inputVariance", mSharedCtx.mResManager->getView(images.variance));
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
