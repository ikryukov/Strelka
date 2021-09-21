#include "bilateralparam.h"

ConstantBuffer<BilateralParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> depth;

Texture2D<float> input;
RWTexture2D<float> output;

#define PI 3.1415926535897

float linearizeDepth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float getWeight(int x, int y)
{
    return (1 / (2 * PI * ubo.sigma * ubo.sigma)) * exp(-((x * x + y * y) / (2 * ubo.sigma * ubo.sigma)));
}

float simpleBlur(uint2 pixelIndex){
    float color = 0.f;
    const int KERNEL_RADIUS = 5;
    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
    {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
        {
            int2 neighbor = pixelIndex + int2(x, y);
            color += input[neighbor];
        }
    }
    color /= (KERNEL_RADIUS * 2 - 1) * (KERNEL_RADIUS * 2 - 1);

    return color;
}

float gaussianBlur2(uint2 pixelIndex) {
    float color = 0.f;
    float curr = 0.f;
    float currDepth = 0.f;
    currDepth = linearizeDepth(depth[pixelIndex.x][pixelIndex.y], ubo.znear, ubo.zfar);
    int KERNEL_RADIUS = lerp(currDepth, 1, ubo.maxR);
    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
    {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
        {
            curr = getWeight(x, y);
            int2 neighbor = pixelIndex + int2(x, y);
            color += curr * input[neighbor];
        }
    }

    return color;
}

float gaussianBlur(uint2 pixelIndex) {
    float color = 0.f;
    float curr = 0.f;
    const int KERNEL_RADIUS = 5;
    for (int x = -ubo.radius; x <= ubo.radius; ++x)
    {
        for (int y = -ubo.radius; y <= ubo.radius; ++y)
        {
            curr = getWeight(x, y);
            int2 neighbor = pixelIndex + int2(x, y);
            color += curr * input[neighbor];
        }
    }

    return color;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    output[pixelIndex] = gaussianBlur2(pixelIndex);
}
