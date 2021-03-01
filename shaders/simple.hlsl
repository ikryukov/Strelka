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
    float4 color;
    float3 ka : COLOR0;
    float3 kd : COLOR1;
    float3 ks : COLOR2;
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
StructuredBuffer<MaterialInput> materials;

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(vi.position, 1.0f));
    out.uv = vi.uv;
    out.normal = vi.normal;
    //out.normal = normalize(mul(modelViewProj,float3(vi.normal, 1.0f)));
    out.materialId = vi.materialId;

    return out;
}


// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   float4 color = float4(materials[inp.materialId].color.rgb, 1.0f);
   float3 ka = float3(materials[inp.materialId].ka.rgb);
   float3 ks= float3(materials[inp.materialId].ks.rgb);
   float3 kd = float3(materials[inp.materialId].kd.rgb);

   return float4(inp.normal, 1.0);
}
