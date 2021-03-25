struct VertexInput
{
    float3 position : POSITION;
    float3 normal;
    float2 uv;
    uint32_t materialId;
};

struct Material
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float3 emissive;
    float opticalDensity;
    float shininess;
    float3 transparency;
    uint32_t illum;
    uint32_t texAmbientId;
    uint32_t texDiffuseId;
    uint32_t texSpeculaId;
    uint32_t texNormalId;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal;
    float2 uv;
    nointerpolation uint32_t materialId;
    float3 wPos;
};

cbuffer ubo
{
    float4x4 modelToWorld;
    float4x4 modelViewProj;
    float4x4 worldToView;
    float4x4 inverseWorldToView;
    float3 CameraPos;
}

Texture2D tex;
SamplerState gSampler;
StructuredBuffer<Material> materials;

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(vi.position, 1.0f));
    out.uv = vi.uv;
    out.normal = normalize(mul((float3x3)inverseWorldToView, vi.normal));
    out.materialId = vi.materialId;
    out.wPos = mul(vi.position, (float3x3)modelToWorld);
    return out;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   // float3 ambient = float3(materials[inp.materialId].ambient.rgb);
   // float3 specular = float3(materials[inp.materialId].specular.rgb);
   // float3 diffuse = float3(materials[inp.materialId].diffuse.rgb);

   float3 emissive = float3(materials[inp.materialId].emissive.rgb);
   float opticalDensity = float(materials[inp.materialId].opticalDensity);
   float shininess = float(materials[inp.materialId].shininess);
   float3 transparency = float3(materials[inp.materialId].transparency.rgb);
   uint32_t illum = 2;

   uint32_t texAmbientId = 0;
   uint32_t texDiffuseId = 0;
   uint32_t texSpeculaId = 0;
   uint32_t texNormalId = 0;

   float3 lightPos = float3(100.0f,100.0f,100.0f);
   float3 N = normalize(inp.normal);
   float3 L = normalize(lightPos - inp.wPos);
   float3 V = normalize(CameraPos - inp.wPos);

   float diffuse = saturate(dot(L, N));
   float3 R = reflect(-L, N);
   float specular = pow(saturate(dot(R, V)), 30);
   float3 output = diffuse + specular;
   return float4(output, 1.0f);
}
