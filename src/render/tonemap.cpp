#include "tonemap.h"

namespace nevk
{
Tonemap::Tonemap(const SharedContext& ctx)
    : ComputePass<Tonemapparam>(ctx)
{
    
}
Tonemap::~Tonemap()
{
}
void Tonemap::initialize()
{
    ComputePass<Tonemapparam>::initialize("shaders/tonemap.hlsl");
}
void Tonemap::setInputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("input", imageView);
}
void Tonemap::setOutputTexture(VkImageView imageView)
{
    mShaderParams.setTexture("output", imageView);
}
} // namespace nevk
