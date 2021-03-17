cbuffer ubo
{
    uint2 dimension;
}
Texture2D input;
SamplerState gSampler;
RWTexture2D output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    output[pixelIndex] = input[pixelIndex];
}
