#include "bilateralparam.h"

ConstantBuffer<BilateralParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float> depth;

Texture2D<float> input;
RWTexture2D<float> output;
RWTexture2D<float> varianceOutput;

#define PI 3.1415926535897

float getWeight(int x, int y)
{
    return (1.0 / (2.0 * PI * ubo.sigma * ubo.sigma)) * exp(-((x * x + y * y) / (2.0 * ubo.sigma * ubo.sigma)));
}

float getWeight(int x, int y, float sigma)
{
    return (1.0 / (2.0 * PI * sigma * sigma)) * exp(-((x * x + y * y) / (2.0 * sigma * sigma)));
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

float gaussianBlur2(uint2 pixelIndex, float var)
{
    float color = 0.f;

    var = clamp(var, 0.0, 0.05);
    var /= 0.05;

    const int KERNEL_RADIUS = lerp(1.0, ubo.maxR, var);
    const float sigma = lerp(0.1, ubo.sigma, var);

    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
    {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
        {
            float weigth = getWeight(x, y, sigma);
            int2 neighbor = pixelIndex + int2(x, y);

            color += weigth * input[neighbor];
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

float variance(uint2 pixelIndex)
{
    float2 sigmaVariancePair = float2(0.0, 0.0);
    float sampCount = 0.0;
    float color = 0.f;
    const int KERNEL_RADIUS = 3;

    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
    {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
        {
            int2 neighbor = pixelIndex + int2(x, y);
            color = input[neighbor];

            // count variance
            float sampSquared = color * color;
            sigmaVariancePair += float2(color, sampSquared);

            sampCount += 1.0;
        }
    }

    sigmaVariancePair /= sampCount;
    float variance = max(0.0, sigmaVariancePair.y - sigmaVariancePair.x * sigmaVariancePair.x);

    return variance;
}

float simpleBilateral(uint2 pixelIndex)
{
    float res = 0;
    const int KERNEL_RADIUS = ubo.radius;
    const int i = pixelIndex.x;
    const int j = pixelIndex.y;
    const float sigma2 = ubo.sigma * ubo.sigma;
    const float sigmaNormal2 = ubo.sigmaNormal * ubo.sigmaNormal;
    float normalization = 1;
    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x)
    {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y)
        {
            int2 neighbor = pixelIndex + int2(x, y);
            if (all(neighbor > 0) && all(neighbor < ubo.dimension))
            {
                // inside image
                float w1 = exp(-(pow(x, 2.0) + pow(y, 2.0)) / (2.0 * sigma2));
                // calculate normal weight
                float w2 = exp(-(dot(gbNormal[pixelIndex].xyz, gbNormal[neighbor].xyz)) / (2.0 * sigmaNormal2));
                res += w1 * w2 * input[neighbor];
                normalization += w1 * w2;
            }
        }
    }
    return res / normalization;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
    {
        output[pixelIndex] = 1.0;
        varianceOutput[pixelIndex] = 0.0;
        return;
    }

    float var = variance(pixelIndex);
    varianceOutput[pixelIndex] = var;
    if (var == 0.0)
    {
        output[pixelIndex] = input[pixelIndex];
        return;
    }

     output[pixelIndex] = simpleBilateral(pixelIndex);
}
