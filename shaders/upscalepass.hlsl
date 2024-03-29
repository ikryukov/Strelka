#include "upscalepassparam.h"

ConstantBuffer<Upscalepassparam> ubo;
Texture2D<float4> input;
RWTexture2D<float4> output;

SamplerState upscaleSampler;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (any(pixelIndex >= ubo.dimension))
    {
        return;
    }

    float2 uv = (pixelIndex + 0.5) * ubo.invDimension;
    float3 color = input.SampleLevel(upscaleSampler, uv, 0).rgb;
    output[pixelIndex] = float4(color, 1.0f);
}
