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
void Composition::setInputTexture(CompositionImages images)
{
    mShaderParams.setTexture("inputLTC", mSharedCtx.mResManager->getView(images.LTC));
    mShaderParams.setTexture("inputShadows", mSharedCtx.mResManager->getView(images.shadow));
    mShaderParams.setTexture("inputAO", mSharedCtx.mResManager->getView(images.AO));
    mShaderParams.setTexture("inputReflections", mSharedCtx.mResManager->getView(images.reflections));
}
void Composition::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
