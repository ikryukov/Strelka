#include "debugviewparam.h"

ConstantBuffer<Debugviewparam> ubo;
Texture2D<float4> inputNormals;
Texture2D<float2> inputMotion;
Texture2D<float4> debugTex;
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
    if (ubo.debugView == 1) // Normals
    {
        output[pixelIndex] = inputNormals[pixelIndex] * 0.5 + 0.5;
    }
    if (ubo.debugView == 2) // Motion
    {
        output[pixelIndex] = float4(abs(inputMotion[pixelIndex]), 0.0, 0.0);
    }
    if (ubo.debugView == 3) // Debug
    {
        output[pixelIndex] = debugTex[pixelIndex];
    }
    if (ubo.debugView == 4) // path tracer
    {
        float3 color = inputPathTracer[pixelIndex].rgb;
        output[pixelIndex] = float4(color, 0.0);
    }
}
