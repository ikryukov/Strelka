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
Texture2D<float2> gbUV;
Texture2D<int> gbInstId;

StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<RectLight> lights;
StructuredBuffer<Material> materials;

Texture2D textures[64]; // bindless
SamplerState samplers[15];

Texture2D<float4> bilateralTexture;
SamplerState bilateralSampler;

RWTexture2D<float4> output;

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    output[pixelIndex] = float4(bilateralTexture[pixelIndex].rgb, 0.0);
}
