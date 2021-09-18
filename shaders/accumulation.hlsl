#include "accumulationparam.h"

ConstantBuffer<AccumulationParam> ubo;
Texture2D<float2> motion;
Texture2D<float> currTex;
Texture2D<float> prevTex;
RWTexture2D<float> output;

float acc(uint2 pixelIndex)
{
    float current = currTex[pixelIndex];
    // pixel coord -> ndc
    float2 currNdc = (2.0 * pixelIndex) / ubo.dimension - 1.0;
    float2 mv = motion[pixelIndex];
    currNdc += mv; // moved to prev ndc
    // prev pixel screen coord
    uint2 prevPixel = ubo.dimension * 0.5 * currNdc + ubo.dimension * 0.5;
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
