#include "accumulationparam.h"

ConstantBuffer<AccumulationParam> ubo;
Texture2D<float2> motion;
Texture2D<float> currTex;
Texture2D<float> prevTex;
RWTexture2D<float> output;


float acc(uint2 pixelIndex)
{
    float current = currTex[pixelIndex];
    float2 mv = motion[pixelIndex];

    float2 currNdc;
    currNdc.x = (2.0 * pixelIndex.x) / ubo.dimension.x - 1.0;
    currNdc.y = (2.0 * pixelIndex.y) / ubo.dimension.y - 1.0;

    currNdc += mv;

    uint2 prevPixel;
    prevPixel.x = ubo.dimension.x * 0.5 * currNdc.x + ubo.dimension.x * 0.5;
    prevPixel.y = ubo.dimension.y * 0.5 * currNdc.y + ubo.dimension.y * 0.5;

    float prev = prevTex[pixelIndex];

    return lerp(prev, current, ubo.alpha);
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    output[pixelIndex] = acc(pixelIndex);
}
