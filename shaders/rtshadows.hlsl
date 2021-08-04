cbuffer ubo
{
    float4x4 viewToProj;
    float4x4 worldToView;
    float4x4 lightSpaceMatrix;
    float4 lightPosition;
    float3 CameraPos;
    float pad0;
    int2 dimension;
    uint32_t debugView;
    float pad1;
}

struct BVHNode 
{
    float3 minBounds;
    int instId;
    float3 maxBounds;
    int nodeOffset;
};

struct Light
{
    float3 pos;
};

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvh;
StructuredBuffer<Light> lights;

RWTexture2D<float> output;

#define INVALID_INDEX -1
#define PI 3.1415926535897

float calcShadow(uint2 pixelIndex)
{
    float3 wpos = gbWPos[pixelIndex].xyz;
    float3 lightPosition = lights[0].pos.xyz;
    float3 L = normalize(lightPosition.xyz - wpos);
    float3 N = gbNormal[pixelIndex].xyz;
    
    float NL = dot(N, L);
    return NL;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    output[pixelIndex] = calcShadow(pixelIndex);
}
