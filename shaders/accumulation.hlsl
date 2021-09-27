#include "accumulationparam.h"

ConstantBuffer<AccumulationParam> ubo;
Texture2D<float4> gbWpos;
Texture2D<float2> motion;
Texture2D<float> currTex;
Texture2D<float> prevTex;
Texture2D<float> prevDepthTex;
Texture2D<float> currDepthTex;
RWTexture2D<float> output;

//RWTexture2D<float4> debug;

float acc(uint2 pixelIndex)
{
    const float current = currTex[pixelIndex];
    // pixel coord -> ndc
    const float2 pixelPos = pixelIndex + 0.5;
    float2 currNdc = (2.0 * pixelPos) / ubo.dimension - 1.0;
    //currNdc.y *= -1.0;
    // prev pixel screen coord
    float2 mv = motion[pixelIndex];
    const float2 prevNdc = currNdc - mv; // moved to prev ndc

    float3 currWpos = gbWpos[pixelIndex].xyz;

    const int2 prevPixel = (prevNdc + 1.0) * ubo.dimension * 0.5; // to screen space
    const float prevDepth = prevDepthTex[prevPixel].r;

    prevNdc.y *= -1.0;

    float4 prevClip = float4(prevNdc, prevDepth, 1.0);
    float4 prevViewSpace = mul(ubo.prevClipToView, prevClip);
    float4 prevWpos = mul(ubo.prevViewToWorld, prevViewSpace);
    prevWpos /= prevWpos.w;
    
    float res = current;
    if (length(prevWpos.xyz - currWpos) < 0.0001)
    {
        // same pixel, reuse sample from history
        float prev = prevTex[prevPixel];
        res = lerp(prev, current, ubo.alpha);
    }
    //return prevDepth;
    return res;
    //return length(prevWpos.xyz - currWpos);
}

float acc1(uint2 pixelIndex)
{
    float2 pixelPos = float2(pixelIndex) + 0.5;
    float2 currNdc = (2.0 * pixelPos) / ubo.dimension - 1.0;
    currNdc.y *= -1.0;
    float currDepth = currDepthTex[pixelIndex].r;
    float4 currClip = float4(currNdc, currDepth, 1.0);

    float4 currViewSpace = mul(ubo.clipToView, currClip);
    float4 currWpos = mul(ubo.viewToWorld, currViewSpace);
    currWpos.xyz /= currWpos.w;

    float3 goldWpos = gbWpos[pixelIndex].xyz;

    float res = length(goldWpos.xyz - currWpos.xyz);
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
    output[pixelIndex] = acc(pixelIndex);
}
