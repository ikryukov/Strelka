#include "debugviewparam.h"

ConstantBuffer<Debugviewparam> ubo;
Texture2D<float4> inputLTC;
Texture2D<float4> inputShadow;
RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = inputLTC[pixelIndex].rgb * inputShadow[pixelIndex].rgb;
    output[pixelIndex] = float4(color, 1.0f);
}
