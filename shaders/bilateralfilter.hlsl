#include "random.h"
#include "materials.h"
#include "lights.h"
#include "bilateralparam.h"

struct InstanceConstants
{
    float4x4 model;
    float4x4 normalMatrix;
    int32_t materialId;
    int32_t pad0;
    int32_t pad1;
    int32_t pad2;
};

ConstantBuffer<BilateralParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> depth;

StructuredBuffer<InstanceConstants> instanceConstants;

Texture2D<float> input;
SamplerState bilateralSampler;

RWTexture2D<float> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float color = 0.f;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
             color += input[pixelIndex.x + x, pixelIndex.y + y];
        }
    }

    color /= 9;
    output[pixelIndex] = color;
}
