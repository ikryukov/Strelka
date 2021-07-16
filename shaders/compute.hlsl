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

Texture2D<float> gbDepth;
Texture2D<float4> gbPos;
Texture2D<float4> gbWPos;
Texture2D<float4> gbPosLightSpace;
Texture2D<float4> gbNormal;
Texture2D<float4> gbTangent;
Texture2D<float4> gbUV;
Texture2D<int> gbInstId;

SamplerState gSampler;

RWTexture2D<float4> output;

struct PointData
{
    float NL;
    float NV;
    float NH;
    float HV;
};

float4 calc(uint2 pixelIndex)
{
    int instId = gbInstId[pixelIndex];
    if (instId < 0)
    {
        return float4(1.0);
    }
    float3 L = normalize(lightPosition.xyz - gbWPos[pixelIndex].xyz);
    float3 N = gbNormal[pixelIndex].xyz;
    
    PointData pointData;
    pointData.NL = dot(N, L);
    return float4(float3(pointData.NL), 1.0);
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    output[pixelIndex] = calc(pixelIndex);
}
