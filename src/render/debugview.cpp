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
void DebugView::setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadow)
{
    std::vector<VkImageView> tex = {imageViewLTC, imageViewShadow};

    mShaderParams.setTextures("input", tex);
}
void DebugView::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
