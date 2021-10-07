#include "compositionparam.h"

ConstantBuffer<Compositionparam> ubo;
Texture2D<float4> inputLTC;
Texture2D<float4> inputShadows;
Texture2D<float4> inputAO;
RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    output[pixelIndex] = float4(inputLTC[pixelIndex].rgb * inputShadows[pixelIndex].r * inputAO[pixelIndex].r, 1.0f);
}
