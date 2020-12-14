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

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color;
};

[shader("vertex")]
PS_INPUT vertexMain(uint vertexID: SV_VertexID)
{
    PS_INPUT out;

    out.pos = float4(positions[vertexID].xy, 0.0, 1.0);
    out.color = float4(colors[vertexID].rgb, 1.0);

    return out;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
    return inp.color;
}
