#include "accumulationparam.h"

ConstantBuffer<AccumulationParam> ubo;
Texture2D<float4> gbWpos;
Texture2D<float2> motionTex;
Texture2D<float4> currTex;
Texture2D<float4> prevTex;
Texture2D<float> prevDepthTex;
Texture2D<float> currDepthTex;
RWTexture2D<float4> output;

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

float3 acc(uint2 pixelIndex)
{
    const float3 current = currTex[pixelIndex].rgb;
    float3 currWpos = gbWpos[pixelIndex].xyz;
    float2 motion = motionTex[pixelIndex].xy;
    float3 prevWpos = reconstructWorldPos(pixelIndex, motion, ubo.dimension, prevDepthTex, ubo.prevClipToView, ubo.prevViewToWorld);
    float3 res = current;
    if (length(prevWpos - currWpos) < 0.01)
    {
        // same pixel, reuse sample from history
        float2 pixelPos = float2(pixelIndex) + 0.5;
        const float2 ndc = 2.0 * (pixelPos - ubo.dimension / 2.0) / ubo.dimension - motion;
        // ndc -> screen
        uint2 prevPixel = (ubo.dimension / 2.0) * ndc + ubo.dimension / 2.0;
        float3 prev = prevTex[prevPixel].rgb;
        res = lerp(prev, current, ubo.alpha);
    }
    return res;
}

float3 acc1(uint2 pixelIndex)
{
    const float3 current = currTex[pixelIndex].rgb;
    if (gbWpos[pixelIndex].w == 0.0)
    {
        return current;
    }
    float3 currWpos = gbWpos[pixelIndex].xyz;
    float4 clip = mul(ubo.prevViewToClip, mul(ubo.prevWorldToView, float4(currWpos, 1.0)));
    float3 ndc = clip.xyz / clip.w;
    int2 prevPixel = (ubo.dimension / 2.0) * ndc.xy + ubo.dimension / 2.0;
    
    float3 res = current;
    if (all(prevPixel >= 0) && all(prevPixel < ubo.dimension))
    {
        const float prevZ = prevDepthTex[prevPixel].r * -1.0 + 1.0;
        if (abs(prevZ - ndc.z) < 0.001)
        {
            // same pixel, reuse sample from history
            float3 prev = prevTex[prevPixel].rgb;
            res = lerp(prev, current, ubo.alpha);
        }
    }
    return res;
}

float3 accPt(uint2 pixelIndex)
{
    const float3 current = currTex[pixelIndex].rgb;
    float3 currWpos = gbWpos[pixelIndex].xyz;
    float2 motion = motionTex[pixelIndex].xy;
    float3 prevWpos = reconstructWorldPos(pixelIndex, motion, ubo.dimension, prevDepthTex, ubo.prevClipToView, ubo.prevViewToWorld);
    float3 res = current;
    if (length(prevWpos - currWpos) < 0.01)
    {
        // same pixel, reuse sample from history
        float2 pixelPos = float2(pixelIndex) + 0.5;
        const float2 ndc = 2.0 * (pixelPos - ubo.dimension / 2.0) / ubo.dimension - motion;
        // ndc -> screen
        uint2 prevPixel = (ubo.dimension / 2.0) * ndc + ubo.dimension / 2.0;
        float3 prev = prevTex[prevPixel].rgb;
        
        float wHistory = float(ubo.iteration) / float(ubo.iteration + 1);
        float wNew = 1.0 / float(ubo.iteration + 1);

        res = prev * wHistory + current * wNew;
    }
    return res;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    output[pixelIndex] = float4(accPt(pixelIndex), 0.0);
}
