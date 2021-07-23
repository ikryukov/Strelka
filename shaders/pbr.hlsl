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

    int32_t sampEmissiveId;
    int32_t texOcclusion;
    int32_t sampOcclusionId;
    int32_t sampBaseId;

    int32_t sampNormalId;
    int32_t pad0;
    int32_t pad1;
    int32_t pad2;
};

struct InstanceConstants
{
    float4x4 model;
    float4x4 normalMatrix;
    int32_t materialId;
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
    int32_t instanceId;
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
SamplerState gSampler[];
StructuredBuffer<Material> materials;
Texture2D shadowMap;
SamplerState shadowSamp;
StructuredBuffer<InstanceConstants> instanceConstants;

static const float4x4 biasMatrix = float4x4(
    0.5, 0.0, 0.0, 0.5, 0.0, -0.5, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

[shader("vertex")] PS_INPUT vertexMain(VertexInput vi) {
    PS_INPUT out;
    InstanceConstants constants = instanceConstants[NonUniformResourceIndex(pconst.instanceId)];
    float4 wpos = mul(constants.model, float4(vi.position, 1.0f));
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

float3 CalcBumpedNormal(PS_INPUT inp, uint32_t texId, uint32_t texSamplerId)
{
    float3 Normal = normalize(inp.normal);
    float3 Tangent = -normalize(inp.tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    float3 Bitangent = cross(Normal, Tangent);

    float3 BumpMapNormal = textures[NonUniformResourceIndex(texId)].Sample(gSampler[texSamplerId], inp.uv).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;

    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    float3 NewNormal = normalize(mul(TBN, BumpMapNormal));

    return NewNormal;
}

#define INVALID_INDEX -1
#define PI 3.1415926535897

float GGX_PartialGeometry(float cosThetaN, float alpha)
{
    float cosTheta_sqr = saturate(cosThetaN * cosThetaN);
    float tan2 = (1.0 - cosTheta_sqr) / cosTheta_sqr;
    float GP = 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * tan2));
    return GP;
}

float GGX_Distribution(float cosThetaNH, float alpha)
{
    float alpha2 = alpha * alpha;
    float NH_sqr = saturate(cosThetaNH * cosThetaNH);
    float den = NH_sqr * alpha2 + (1.0 - NH_sqr);
    return alpha2 / (PI * den * den);
}

float3 FresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - saturate(cosTheta), 5.0);
}

struct PointData
{
    float NL;
    float NV;
    float NH;
    float HV;
};

float3 cookTorrance(in Material material, in PointData pd, in float2 uv)
{
    if (pd.NL <= 0.0 || pd.NV <= 0.0)
    {
        return float3(0.0, 0.0, 0.0);
    }

    float roughness = material.roughnessFactor;
    float roughness2 = roughness * roughness;

    float G = GGX_PartialGeometry(pd.NV, roughness2) * GGX_PartialGeometry(pd.NL, roughness2);
    float D = GGX_Distribution(pd.NH, roughness2);

    float3 f0 = float3(0.24, 0.24, 0.24);
    float3 F = FresnelSchlick(f0, pd.HV);
    //mix
    float3 specK = G * D * F * 0.25 / pd.NV;
    float3 diffK = saturate(1.0 - F);

    float3 albedo = material.baseColorFactor.rgb;
    if (material.texBaseColor != INVALID_INDEX)
    {
        albedo *= textures[NonUniformResourceIndex(material.texBaseColor)].Sample(gSampler[material.sampBaseId], uv).rgb;
    }


    float3 result = max(0.0, albedo * diffK * pd.NL / PI + specK);
    return result;
}

// Fragment Shader
[shader("fragment")] float4 fragmentMain(PS_INPUT inp)
    : SV_TARGET
{
    InstanceConstants constants = instanceConstants[NonUniformResourceIndex(pconst.instanceId)];
    Material material = materials[NonUniformResourceIndex(constants.materialId)];

    int32_t texNormalId = material.texNormalId;

    float3 N = normalize(inp.normal);
    if (texNormalId != INVALID_INDEX)
    {
        N = CalcBumpedNormal(inp, texNormalId, material.sampNormalId);
    }
    float3 L = normalize(lightPosition.xyz - inp.wPos);

    PointData pointData;

    pointData.NL = dot(N, L);

    float3 V = normalize(CameraPos - inp.wPos);
    pointData.NV = dot(N, V);

    float3 H = normalize(V + L);
    pointData.HV = dot(H, V);
    pointData.NH = dot(N, H);

    float3 result = cookTorrance(material, pointData, inp.uv);

    if (material.texEmissive != INVALID_INDEX)
    {
        float3 emissive = textures[NonUniformResourceIndex(material.texEmissive)].Sample(gSampler[material.sampEmissiveId], inp.uv).rgb;
        result += emissive;
    }
    if (material.texOcclusion != INVALID_INDEX)
    {
        float occlusion = textures[NonUniformResourceIndex(material.texOcclusion)].Sample(gSampler[material.sampOcclusionId], inp.uv).r;
        result *= occlusion;
    }
    float shadow = ShadowCalculation(inp.posLightSpace);

    // Normals
    if (debugView == 1)
    {
        return float4(abs(N), 1.0);
    }
    // Shadow b&w
    if (debugView == 2)
    {
        return float4(dot(N, L) * shadow, 1.0);
    }
    // pcf shadow
    if (debugView == 3)
    {
        shadow = ShadowCalculationPcf(inp.posLightSpace);
    }
    // poisson shadow
    if (debugView == 4)
    {
        shadow = ShadowCalculationPoisson(inp.posLightSpace, inp.wPos);
    }
    // poisson + pcf shadow
    if (debugView == 5)
    {
        shadow = ShadowCalculationPoissonPCF(inp.posLightSpace, inp.wPos);
    }

    result *= shadow;

    float alpha = material.d;

    return float4(float3(result), alpha);
}
