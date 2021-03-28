struct VertexInput
{
    float3 position : POSITION;
    uint32_t normal;
    uint32_t uv;
    uint16_t materialId;
};

struct Material
{
  float4 ambient; // Ka
  float4 diffuse; // Kd
  float4 specular; // Ks
  float4 emissive; // Ke
  float4 transparency; //  d 1 -- прозрачность/непрозрачность
  float opticalDensity; // Ni
  float shininess; // Ns 16 --  блеск материала
  uint32_t illum; // illum 2 -- модель освещения
  uint32_t texDiffuseId; // map_diffuse

  uint32_t texAmbientId; // map_ambient
  uint32_t texSpecularId; // map_specular
  uint32_t texNormalId; // map_normal - map_Bump
  uint32_t pad;
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

Texture2D textures[128];
SamplerState gSampler;
StructuredBuffer<Material> materials;

float3 unpackNormal(uint32_t val)
{
   float3 normal;
   normal.z = ((val & 0xfff00000) >> 20) / 511.99999f * 2.0f - 1.0f;
   normal.y = ((val & 0x000ffc00) >> 10) / 511.99999f * 2.0f - 1.0f;
   normal.x = (val & 0x000003ff) / 511.99999f * 2.0f - 1.0f;

   return normal;
}

float2 unpackUV(uint32_t val)
{
   float2 uv;
   uv.y = ((val & 0xffff0000) >> 16) / 16383.99999f * 2.0f - 1.0f;
   uv.x = (val & 0x0000ffff) / 16383.99999f * 2.0f  - 1.0f;

   return uv;
}
[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(vi.position, 1.0f));
    out.uv = unpackUV(vi.uv);
    out.normal = mul((float3x3)inverseWorldToView, unpackNormal(vi.normal));
    out.materialId = vi.materialId;

    return out;
}


// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   float3 ambient = float3(materials[inp.materialId].ambient.rgb);
   float3 specular = float3(materials[inp.materialId].specular.rgb);
   float3 diffuse = float3(materials[inp.materialId].diffuse.rgb);

   float3 emissive = float3(materials[inp.materialId].emissive.rgb);
   float opticalDensity = float(materials[inp.materialId].opticalDensity);
   float shininess = float(materials[inp.materialId].shininess);
   float3 transparency = float3(materials[inp.materialId].transparency.rgb);
   uint32_t illum = 2;

   uint32_t texAmbientId = materials[inp.materialId].texAmbientId;
   uint32_t texDiffuseId = materials[inp.materialId].texDiffuseId;
   uint32_t texSpecularId = materials[inp.materialId].texSpecularId;
   uint32_t texNormalId = materials[inp.materialId].texNormalId;

   return textures[texDiffuseId].Sample(gSampler, inp.uv);
}
