#include "pathtracerparam.h"

ConstantBuffer<PathTracerParam> ubo;

StructuredBuffer<float> sampleBuffer;
// StructuredBuffer<float> compositingBuffer;
RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 dispatchIndex : SV_DispatchThreadID)
{
    if (dispatchIndex.x >= ubo.dimension.x * ubo.dimension.y)
    {
        return;
    }
    int pixelIndex = dispatchIndex.x;
    float3 color = float3(0.0f);
    float invSpp = 1.0f / ubo.spp;
    for (int sample = 0; sample < ubo.spp; ++sample)
    {
        color.r += invSpp * sampleBuffer[(pixelIndex * ubo.spp + sample) * 3 + 0];
        color.g += invSpp * sampleBuffer[(pixelIndex * ubo.spp + sample) * 3 + 1];
        color.b += invSpp * sampleBuffer[(pixelIndex * ubo.spp + sample) * 3 + 2];
    }

    // compositingBuffer[pixelIndex * 3 + 0] = color.r;
    // compositingBuffer[pixelIndex * 3 + 1] = color.g;
    // compositingBuffer[pixelIndex * 3 + 2] = color.b;
    int2 pixelCoord = int2(dispatchIndex.x % ubo.dimension.x, dispatchIndex.x / ubo.dimension.x);
    output[pixelCoord] = float4(color, 1.0f);
}
