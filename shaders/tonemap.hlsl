cbuffer ubo
{
    uint2 dimension;
}
Texture2D input;
SamplerState gSampler;
RWTexture2D<float4> output;

float3 ToneMapFilmicALU(in float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);
    return color;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void tonemapMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    float3 color = input[pixelIndex.xy].rgb;
	float4 tonmappedColor = float4(ToneMapFilmicALU(color).xyz, 1.0f);
    output[pixelIndex] = input[pixelIndex];
	output[pixelIndex.xy] = tonmappedColor;
}
