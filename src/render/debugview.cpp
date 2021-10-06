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
void DebugView::setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadow, VkImageView imageNormal, VkImageView imageMotion, VkImageView debug, VkImageView imageAO)
{
    mShaderParams.setTexture("inputLTC", imageViewLTC);
    mShaderParams.setTexture("inputShadow", imageViewShadow);
    mShaderParams.setTexture("inputNormals", imageNormal);
    mShaderParams.setTexture("inputMotion", imageMotion);
    mShaderParams.setTexture("debugTex", debug);
    mShaderParams.setTexture("inputAO", imageAO);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
