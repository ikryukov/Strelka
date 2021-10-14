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
    mShaderParams.setTexture("inputShadow", imageViewShadow);
    mShaderParams.setTexture("inputNormals", imageNormal);
    mShaderParams.setTexture("inputMotion", imageMotion);
    mShaderParams.setTexture("debugTex", debug);
    mShaderParams.setTexture("inputAO", imageViews.AO);
    mShaderParams.setTexture("inputVariance", imageVariance);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
