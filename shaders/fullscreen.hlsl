static const float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

static const float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
};

Texture2D tex;
SamplerState gSampler;

struct VertexInput
{
  uint VertexID : SV_VertexID;
  float2 uv;
};

struct PixelInput
{
  float4 Position : SV_POSITION;
  float2 uv;
};

float3 ToneMapFilmicALU(in float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);
    return color;
}
 
PixelInput DefaultVS(VertexInput input)
{
  PixelInput output = (PixelInput)0;

  uint id = input.VertexID;

  float x = -1, y = -1;
  x = (id == 2) ? 3.0 : -1.0;
  y = (id == 1) ? 3.0 : -1.0;

  output.Position = float4(x, y, 1.0, 1.0);
  output.uv = input.uv;

  return output;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PixelInput inp) : SV_TARGET
{
	float3 tonmappedColor = ToneMapFilmicALU(tex.Sample(gSampler, inp.uv).rgb);
    float4 resultColor = float4(tonmappedColor, 1.0f);

    return resultColor;
}
