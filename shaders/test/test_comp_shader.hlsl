struct Ubo
{
    float4x4 viewToProj;
    float4x4 worldToView;
    float3 CameraPos;
    float pad0;
    int2 dimension;
    uint32_t debugView;
    float pad1;
}

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

struct Light
{
    float3 v0;
    float pad0;
    float3 v1;
    float pad1;
    float3 v2;
    float pad2;
};

struct Gbuffer {
    Texture2D<float> depth;
    Texture2D<float4> wPos;
    Texture2D<float4> normal;
    Texture2D<float4> tangent;
    Texture2D<float2> uv;
    Texture2D<int> instId;
    StructuredBuffer<Material> materials;
};
ParameterBlock<Gbuffer> gGbuffer;

struct MyStruct
{
    float4x4 mat;
    int2 dim;
    uint32_t a;
    StructuredBuffer<Material> materials;
    Texture2D<float4> wPos;
    SamplerState mySampler;
    RWTexture2D<float4> myOutput;
};
ConstantBuffer<Ubo> gUbo;

SamplerState gSampler;

Texture2D textures[64]; // bindless


StructuredBuffer<Material> materials;
StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Light> lights;

Texture2D<float> shadow;

RWTexture2D<float4> output;

struct PointData
{
    float NL;
    float NV;
    float NH;
    float HV;
};

#define INVALID_INDEX -1
#define PI 3.1415926535897

float4 calc(uint2 pixelIndex)
{
    int instId = gGbuffer.instId[pixelIndex];
    if (instId < 0)
    {
        return float4(1.0);
    }
    InstanceConstants constants = instanceConstants[NonUniformResourceIndex(instId)];
    Material material = materials[NonUniformResourceIndex(constants.materialId)];

    float3 wpos = gGbuffer.wPos[pixelIndex].xyz;
    float3 L = normalize(lights[0].v0.xyz - wpos);
    float3 N = gGbuffer.normal[pixelIndex].xyz;
    
    PointData pointData;
    pointData.NL = dot(N, L);
    float3 V = normalize(gUbo.CameraPos - wpos);
    pointData.NV = dot(N, V);
    float3 H = normalize(V + L);
    pointData.HV = dot(H, V);
    pointData.NH = dot(N, H);
    float2 uv = gGbuffer.uv[pixelIndex].xy;

    float3 result = float3(pointData.NL);

    result *= shadow[pixelIndex];

    return float4(result, 1.0);
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= gUbo.dimension.x || pixelIndex.y >= gUbo.dimension.y)
    {
        return;
    }
    output[pixelIndex] = calc(pixelIndex);
}
