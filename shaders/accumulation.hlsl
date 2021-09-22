#include "accumulationparam.h"

ConstantBuffer<AccumulationParam> ubo;
Texture2D<float4> gbWpos;
Texture2D<float2> motion;
Texture2D<float> currTex;
Texture2D<float> prevTex;
Texture2D<float> prevDepthTex;
RWTexture2D<float> output;

float acc(uint2 pixelIndex)
{
    const float current = currTex[pixelIndex];
    // pixel coord -> ndc
    const float2 currNdc = (2.0 * pixelIndex) / ubo.dimension - 1.0;
    // prev pixel screen coord
    float2 mv = motion[pixelIndex];
    const float2 prevNdc = currNdc - mv; // moved to prev ndc

    float3 currWpos = gbWpos[pixelIndex].xyz;
    // float4 currPosInPrev = mul(ubo.prevViewToProj, mul(ubo.prevWorldToView, float4(currWpos, 1.0))); // reproject current pos using prev matrices
    // float currDepthInPrev = currPosInPrev.z / currPosInPrev.w; // depth in NDC

    const uint2 prevPixel = (currNdc + 1.0) * ubo.dimension * 0.5; // to screen space
    const float prevDepth = prevDepthTex[prevPixel].r;

    float4 prevClip = float4(prevNdc, prevDepth, 1.0);
    float4 prevViewSpace = mul(ubo.prevProjToView, prevClip);
    prevViewSpace /= prevViewSpace.w;
    float4 prevWpos = mul(ubo.prevViewToWorld, prevViewSpace);
    
    float res = current;
    //if (length(prevWpos.xyz - currWpos) < 1e-5)
    {
        // found same pixel, reuse sample
        float prev = prevTex[prevPixel];
        res = lerp(prev, current, ubo.alpha);
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
    output[pixelIndex] = acc(pixelIndex);
}
