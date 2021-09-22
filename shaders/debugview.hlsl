#include "debugviewparam.h"

ConstantBuffer<Debugviewparam> ubo;
Texture2D<float4> inputLTC;
Texture2D<float4> inputShadow;
Texture2D<float4> inputNormals;
Texture2D<float4> inputVariance;
RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    if (ubo.debugView == 1) // Normals
    {
        output[pixelIndex] = inputNormals[pixelIndex] * 0.5 + 0.5;
    }
    if (ubo.debugView == 2) // b&w shadows
    {
        float3 color = inputShadow[pixelIndex].r;
        output[pixelIndex] = float4(color, 0.0);
    }
    if (ubo.debugView == 3) // LTC
    {
        float3 color = inputLTC[pixelIndex].rgb;
        output[pixelIndex] = float4(color, 0.0);
    }
    if (ubo.debugView == 4) // variance
    {
    float3 color = inputVariance[pixelIndex].r;
    output[pixelIndex] = float4(color, 0.0);
    }
}
