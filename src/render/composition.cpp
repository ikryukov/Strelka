#include "composition.h"

namespace nevk
{
Composition::Composition(const SharedContext& ctx)
    : CompositionBase(ctx)
{

}
Composition::~Composition()
{
}
void Composition::initialize()
{
    CompositionBase::initialize("shaders/composition.hlsl");
}
void Composition::setInputTexture(VkImageView imageViewLTC, VkImageView imageViewShadows, VkImageView imageViewAO)
{
    mShaderParams.setTexture("inputLTC", imageViewLTC);
    mShaderParams.setTexture("inputShadows", imageViewShadows);
    mShaderParams.setTexture("inputAO", imageViewAO);
}
void Composition::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk