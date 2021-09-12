#include "Tonemapparam.h"

ConstantBuffer<Tonemapparam> ubo;
Texture2D<float4> input;
RWTexture2D<float4> output;

float4 calc(uint2 pixelIndex)
{
    return input[pixelIndex];
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    output[pixelIndex] = calc(pixelIndex);
}
