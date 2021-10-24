#include "compositionparam.h"

ConstantBuffer<Compositionparam> ubo;
Texture2D<float4> inputLTC;
Texture2D<float4> inputShadows;
Texture2D<float4> inputAO;
Texture2D<float4> inputReflections;
Texture2D<float4> inputPathTracer;
RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = float3(0.f);
    if (ubo.enablePathTracer) {
        color = inputPathTracer[pixelIndex].rgb;
    }
    else
    {
        color = inputLTC[pixelIndex].rgb;

        if (ubo.enableReflections)
        {
            color += inputReflections[pixelIndex].rgb;
        }
        if (ubo.enableAO)
        {
            color *= inputAO[pixelIndex].r;
        }
        if (ubo.enableShadows)
        {
            color *= inputShadows[pixelIndex].r;
        }
    }

    output[pixelIndex] = float4(color, 1.0f);
}
