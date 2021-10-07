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
void DebugView::setInputTexture(DebugImageViews imageViews)
{
    mShaderParams.setTexture("inputLTC", imageViews.imageViewLTC);
    mShaderParams.setTexture("inputShadow", imageViews.imageViewShadow);
    mShaderParams.setTexture("inputNormals", imageViews.imageNormal);
    mShaderParams.setTexture("inputMotion", imageViews.imageMotion);
    mShaderParams.setTexture("debugTex", imageViews.debug);
    mShaderParams.setTexture("inputAO", imageViews.imageAO);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
