#include "Tonemapparam.h"

ConstantBuffer<Tonemapparam> ubo;
Texture2D<float4> inputLTC;
Texture2D<float4> inputShadows;
RWTexture2D<float4> output;

// original implementation https://github.com/NVIDIAGameWorks/Falcor/blob/5236495554f57a734cc815522d95ae9a7dfe458a/Source/RenderPasses/ToneMapper/ToneMapping.ps.slang

float calcLuminance(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

// Reinhard
float3 toneMapReinhard(float3 color)
{
    float luminance = calcLuminance(color);
    float reinhard = luminance / (luminance + 1);
    return color * (reinhard / luminance);
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    float3 color = inputLTC[pixelIndex].rgb;
    output[pixelIndex] = float4(toneMapReinhard(color) * inputShadows[pixelIndex].r, 1.0f);
}
