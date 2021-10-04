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
void DebugView::setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadow, VkImageView imageNormal, VkImageView imageMotion, VkImageView debug)
void DebugView::setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadow, VkImageView imageNormal, VkImageView imageVariance)
{
    mShaderParams.setTexture("inputLTC", imageViewLTC);
    mShaderParams.setTexture("inputShadow", imageViewShadow);
    mShaderParams.setTexture("inputNormals", imageNormal);
    mShaderParams.setTexture("inputMotion", imageMotion);
    mShaderParams.setTexture("debugTex", debug);
    mShaderParams.setTexture("inputVariance", imageVariance);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
