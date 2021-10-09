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
    mShaderParams.setTexture("inputLTC", imageViews.LTC);
    mShaderParams.setTexture("inputShadow", imageViews.shadow);
    mShaderParams.setTexture("inputNormals", imageViews.normal);
    mShaderParams.setTexture("inputMotion", imageViews.motion);
    mShaderParams.setTexture("debugTex", imageViews.debug);
    mShaderParams.setTexture("inputAO", imageViews.AO);
    mShaderParams.setTexture("inputReflection", imageViews.reflection);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
