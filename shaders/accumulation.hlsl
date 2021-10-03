#include "accumulationparam.h"

ConstantBuffer<AccumulationParam> ubo;
Texture2D<float4> gbWpos;
Texture2D<float2> motionTex;
Texture2D<float> currTex;
Texture2D<float> prevTex;
Texture2D<float> prevDepthTex;
Texture2D<float> currDepthTex;
RWTexture2D<float> output;

float3 reconstructWorldPos(uint2 pixelIndex, float2 motion, uint2 dimension, Texture2D<float> depthTex, float4x4 clipToView, float4x4 viewToWorld)
{
    float2 pixelPos = float2(pixelIndex) + 0.5;
    // https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#vertexpostproc-viewport
    // xf = (px / 2) * xd + ox
    // px = viewport width
    // xd = (xf - ox) / (px / 2)
    // xd = 2 * (xf - ox) / px
    // ox = ubo.dimension / 2
    // ndc = 2 * (pixelPos - ubo.dimension / 2.0) / ubo.dimension
    const float2 ndc = 2.0 * (pixelPos - dimension / 2.0) / dimension - motion;
    // zf = pz * zd + oz
    // zd = (zf - oz) / pz;
    // pz - range scale depth, oz - range bias depth
    const float depth = depthTex[pixelIndex].r * -1.0 + 1.0;
    float4 clip = float4(ndc, depth, 1.0);

    float4 viewSpace = mul(clipToView, clip);
    float4 wpos = mul(viewToWorld, viewSpace);
    wpos /= wpos.w;
    return wpos.xyz;
}

float acc(uint2 pixelIndex)
{
    const float current = currTex[pixelIndex];
    float3 currWpos = gbWpos[pixelIndex].xyz;
    float2 motion = motionTex[pixelIndex].xy;
    float3 prevWpos = reconstructWorldPos(pixelIndex, motion, ubo.dimension, prevDepthTex, ubo.prevClipToView, ubo.prevViewToWorld);
    
    float res = current;
    if (length(prevWpos - currWpos) < 0.01)
    {
        // same pixel, reuse sample from history
        float2 pixelPos = float2(pixelIndex) + 0.5;
        const float2 ndc = 2.0 * (pixelPos - ubo.dimension / 2.0) / ubo.dimension - motion;
        // ndc -> screen
        uint2 prevPixel = (ubo.dimension / 2.0) * ndc + ubo.dimension / 2.0;

        float prev = prevTex[prevPixel];
        res = lerp(prev, current, ubo.alpha);
    }
    //return prevDepth;
    return res;
    //return length(prevWpos.xyz - currWpos);
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    output[pixelIndex] = acc(pixelIndex);
}
