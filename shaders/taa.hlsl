struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv;
};

//===================================
// Descriptor layouts
Texture2D gTexture;
SamplerState gSampler;
//===================================

[shader("vertex")]
PS_INPUT vertexMain(uint VertexID: SV_VertexID)
{
    PS_INPUT out;

    out.uv = float2((VertexID << 1) & 2, VertexID & 2);
    out.pos = float4(out.uv * float2(1.0f, -1.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

    return out;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
    return float4(gTexture.Sample(gSampler, inp.uv));
}
