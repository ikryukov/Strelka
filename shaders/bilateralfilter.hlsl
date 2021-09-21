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
    const int KERNEL_RADIUS = 5;
    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
    {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
        {
            int2 neighbor = pixelIndex + int2(x, y);
            color += input[neighbor];
        }
    }

    color /= (KERNEL_RADIUS * 2 - 1) * (KERNEL_RADIUS * 2 - 1);
    output[pixelIndex] = color;
}
