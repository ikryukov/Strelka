#include "bilateralparam.h"

ConstantBuffer<BilateralParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> depth;

Texture2D<float> input;
RWTexture2D<float> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float color = 0.f;
    int2 neighbour = 0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
             neighbour = pixelIndex + int2(x, y);
             color += input[neighbour];
        }
    }

    color /= 9;
    output[pixelIndex] = color;
}
