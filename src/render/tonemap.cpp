#include "tonemap.h"

namespace nevk
{
Tonemap::Tonemap(const SharedContext& ctx)
    : TonemapBase(ctx)
{
    
}
Tonemap::~Tonemap()
{
}
void Tonemap::initialize()
{
    TonemapBase::initialize("shaders/tonemap.hlsl");
}
void Tonemap::setInputTexture(VkImageView compositionImageView)
{
    mShaderParams.setTexture("input", compositionImageView);
}
void Tonemap::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
