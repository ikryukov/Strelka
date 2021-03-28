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
struct AssembledVertex
{
    float3  position : POSITION;
    float3  ka   : COLOR0;
    float3  ks   : COLOR1;
    float3  kd   : COLOR2;
    float2  uv;
};

cbuffer ubo
{
    float4x4 modelViewProj;
}

float3 ToneMapFilmicALU(in float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f)+ 0.06f);
    return color;
}

Texture2D tex;
SamplerState gSampler;

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 ka;
    float4 ks;
    float4 kd;
    float2 uv;
};

[shader("vertex")]
PS_INPUT vertexMain(AssembledVertex av)
{
    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(av.position, 1.0f));
    out.ka = float4(av.ka.rgb, 1.0f);
    out.ks = float4(av.ks.rgb, 1.0f);
    out.kd = float4(av.kd.rgb, 1.0f);
    out.uv = av.uv;

    return out;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
    float3 tonmappedColor = ToneMapFilmicALU(inp.ka.rgb + (inp.ks.rgb + inp.kd.rgb) * tex.Sample(gSampler, inp.uv).rgb);
    float4 resultColor = float4(tonmappedColor, 1.0f);

    return resultColor;
}
