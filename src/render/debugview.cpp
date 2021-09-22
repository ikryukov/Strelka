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
void DebugView::setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadow, VkImageView imageNormal, VkImageView imageMotion)
{
    mShaderParams.setTexture("inputLTC", imageViewLTC);
    mShaderParams.setTexture("inputShadow", imageViewShadow);
    mShaderParams.setTexture("inputNormals", imageNormal);
    mShaderParams.setTexture("inputMotion", imageMotion);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
