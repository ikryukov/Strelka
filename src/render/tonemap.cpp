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
} // namespace nevk
