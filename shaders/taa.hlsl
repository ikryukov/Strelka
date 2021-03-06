struct VertexInput
{
    float3 position : POSITION;
    float3 normal;
    float2 uv;
    uint32_t materialId;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal;
    float2 uv;
    nointerpolation uint32_t materialId;
};

//===================================
// Descriptor layouts
Texture2D gTexture;
SamplerState gSampler;
//===================================

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = float4(vi.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    out.uv = vi.uv;
    out.normal = vi.normal;
    out.materialId = vi.materialId;

    return out;
}

[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
    return float4(gTexture.Sample(gSampler, inp.uv));
}
