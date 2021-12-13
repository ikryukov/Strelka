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
    mShaderParams.setTexture("inputNormals", mSharedCtx.mResManager->getView(images.normal));
    mShaderParams.setTexture("inputMotion", mSharedCtx.mResManager->getView(images.motion));
    mShaderParams.setTexture("debugTex", mSharedCtx.mResManager->getView(images.debug));
    mShaderParams.setTexture("inputPathTracer", mSharedCtx.mResManager->getView(images.pathTracer));
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
