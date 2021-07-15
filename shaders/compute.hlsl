cbuffer ubo
{
    uint2 dimension;
}

Texture2D<float> gbDepth;
Texture2D<float4> gbPos;
Texture2D<float4> gbWPos;
Texture2D<float4> gbPosLightSpace;
Texture2D<float4> gbNormal;
Texture2D<float4> gbTangent;
Texture2D<float4> gbUV;
Texture2D<int> gbInstId;

SamplerState gSampler;

RWTexture2D output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    output[pixelIndex] = gbWPos[pixelIndex];
}
