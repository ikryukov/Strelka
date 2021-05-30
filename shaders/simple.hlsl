#include "pack.h"
#include "shadow.h"

struct VertexInput
{
    float3 position : POSITION;
    uint32_t tangent;
    uint32_t normal;
    uint32_t uv;
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
    int32_t texDiffuseId; // map_diffuse

    int32_t texAmbientId; // map_ambient
    int32_t texSpecularId; // map_specular
    int32_t texNormalId; // map_normal - map_Bump
    float d; // alpha value
    //====PBR====
    float4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;
    int32_t metallicRoughnessTexture;
    int32_t texBaseColor;

    float3 emissiveFactor;
    int32_t texEmissive;
    int32_t texOcclusion;
    int32_t pad0;
    int32_t pad1;
    int32_t pad2;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 posLightSpace;
    float3 tangent;
    float3 normal;
    float3 wPos;
    float2 uv;
};

struct InstancePushConstants 
{
    float4x4 model;
    int32_t materialId;
};
[[vk::push_constant]] ConstantBuffer<InstancePushConstants> pconst;

cbuffer ubo
{
    float4x4 viewToProj;
    float4x4 worldToView;
    float4x4 lightSpaceMatrix;
    float4 lightPosition;
    float3 CameraPos;
    float pad;
    uint32_t debugView;
}

Texture2D textures[];
SamplerState gSampler;
StructuredBuffer<Material> materials;
Texture2D shadowMap;
SamplerState shadowSamp;

static const float4x4 biasMatrix = float4x4(
  0.5, 0.0, 0.0, 0.5,
  0.0, -0.5, 0.0, 0.5,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0 );

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    float4 wpos = mul(pconst.model, float4(vi.position, 1.0f));
    out.pos = mul(viewToProj, mul(worldToView, wpos));
    out.posLightSpace = mul(biasMatrix, mul(lightSpaceMatrix, wpos));
    out.uv = unpackUV(vi.uv);
    // assume that we don't use non-uniform scales
    // TODO:
    out.normal = unpackNormal(vi.normal);
    out.tangent = unpackTangent(vi.tangent);
    out.wPos = wpos.xyz / wpos.w; 
    return out;
}

float3 diffuseLambert(float3 kD, float3 n, float3 l)
{
    return kD * saturate(dot(l, n));
}

float3 specularPhong(float3 kS, float3 r, float3 v, float shinessFactor)
{
    return kS * pow(saturate(dot(r, v)), shinessFactor);
}

float3 CalcBumpedNormal(PS_INPUT inp, uint32_t texId)
{
    float3 Normal = normalize(inp.normal);
    float3 Tangent = -normalize(inp.tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    float3 Bitangent = cross(Normal, Tangent);

    float3 BumpMapNormal = textures[NonUniformResourceIndex(texId)].Sample(gSampler, inp.uv).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;

    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    float3 NewNormal = normalize(mul(TBN, BumpMapNormal));

    return NewNormal;
}

#define INVALID_INDEX -1

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   Material material = materials[NonUniformResourceIndex(pconst.materialId)];

   float3 emissive = float3(material.emissive.rgb);
   float opticalDensity = material.opticalDensity;
   float shininess = material.shininess;
   float3 transparency = float3(material.transparency.rgb);
   uint32_t illum = material.illum;
   float d = material.d;

   uint32_t texAmbientId = material.texAmbientId;
   uint32_t texDiffuseId = material.texDiffuseId;
   uint32_t texSpecularId = material.texSpecularId;
   uint32_t texNormalId = material.texNormalId;

   float3 kA = material.ambient.rgb;
   float3 kD = material.diffuse.rgb;
   float3 kS = material.specular.rgb;

   if (texAmbientId != INVALID_INDEX)
   {
      kA *= textures[NonUniformResourceIndex(texAmbientId)].Sample(gSampler, inp.uv).rgb;
   }
   if (texDiffuseId != INVALID_INDEX)
   {
      kD *= textures[NonUniformResourceIndex(texDiffuseId)].Sample(gSampler, inp.uv).rgb;
   }
   if (texSpecularId != INVALID_INDEX)
   {
      kS *= textures[NonUniformResourceIndex(texSpecularId)].Sample(gSampler, inp.uv).rgb;
   }

   float3 N = normalize(inp.normal);
   if (texNormalId != INVALID_INDEX)
   {
      N = CalcBumpedNormal(inp, texNormalId);
   }

   float3 L = normalize(lightPosition.xyz - inp.wPos);
   float3 diffuse = diffuseLambert(kD, L, N);

   float3 R = reflect(-L, N);
   float3 V = normalize(CameraPos - inp.wPos);
   float3 specular = specularPhong(kS, R, V, shininess);

   // Normals
   if (debugView == 1)
   {
      return float4(abs(N), d);
   }
   // Shadow b&w
   if (debugView == 2)
   {
       float shadow = ShadowCalculation(inp.posLightSpace);
       return float4(dot(N, L) * shadow, d);
   }
   // pcf shadow
   if (debugView == 3)
   {
        float shadow = ShadowCalculationPcf(inp.posLightSpace);
        return float4(saturate(kA + diffuse + specular) * shadow, d);
   }
   // poisson shadow
   if (debugView == 4)
   {
        float shadow = ShadowCalculationPoisson(inp.posLightSpace, inp.wPos);
        return float4(saturate(kA + diffuse + specular) * shadow, d);
   }
    // poisson + pcf shadow
    if (debugView == 5)
    {
        float shadow = ShadowCalculationPoissonPCF(inp.posLightSpace, inp.wPos);
        return float4(saturate(kA + diffuse + specular) * shadow, d);
    }

    float shadow = ShadowCalculation(inp.posLightSpace);

    return float4(saturate(kA + diffuse + specular) * shadow, d);
}
