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
    float3 ambient : COLOR0;
    float3 diffuse : COLOR1;
    float3 specular : COLOR2;
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
    float4x4 worldToView;
    float4x4 inverseWorldToView;
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
    out.normal = mul((float3x3)modelViewProj, vi.normal);
    out.materialId = vi.materialId;

    return out;
}


// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   float4 color = float4(materials[inp.materialId].color.rgb, 1.0f);
   float3 ambient = float3(materials[inp.materialId].ambient.rgb);
   float3 specular = float3(materials[inp.materialId].specular.rgb);
   float3 diffuse = float3(materials[inp.materialId].diffuse.rgb);

   return float4(inp.normal, 1.0);
}
