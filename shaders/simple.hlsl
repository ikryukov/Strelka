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
struct VertexInput
{
    float3 position : POSITION;
    float3 normal;
    float2 uv;
    uint32_t materialId;
};

struct MaterialInput
{
    float4 color : COLOR;
    float3 ka : COLOR0;
    float3 kd : COLOR1;
    float3 ks : COLOR2;
};

struct M_INPUT
{
    float4 color;
    float3 ka;
    float3 kd;
    float3 ks;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal;
    float2 uv;
    nointerpolation uint32_t materialId;
};

cbuffer ubo
{
    float4x4 modelViewProj;
}
Texture2D tex;
SamplerState gSampler;

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(vi.position, 1.0f));
    out.uv = vi.uv;
    out.normal = vi.normal;
    out.materialId = vi.materialId;
    //out.materialId = 0;

    return out;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp, M_INPUT m_inp) : SV_TARGET
{
    StructuredBuffer<MaterialInput> materials;
    MaterialInput mater = materials[materialId];
    m_inp.color = mater.color;
    m_inp.ka = mater.ka;
    m_inp.ks = mater.ks;
    m_inp.kd = mater.kd;

    return float4(inp.normal, 1.0);
}
