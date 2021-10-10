#include "lights.h"
#include "reflectionparam.h"
#include "raytracing.h"
#include "materials.h"
#include "pack.h"

ConstantBuffer<ReflectionParam> ubo;

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;
Texture2D<float4> instId;

StructuredBuffer<BVHNode> bvhNodes;
StructuredBuffer<BVHTriangle> bvhTriangles;
StructuredBuffer<RectLight> lights;

RWTexture2D<float4> output;

struct InstanceConstants
{
    float4x4 model;
    float4x4 normalMatrix;
    int32_t materialId;
    int32_t ibId;
    int32_t vbId;
    int32_t pad0;
};

StructuredBuffer<InstanceConstants> instanceConstants;
StructuredBuffer<Material> materials;

Texture2D textures[64]; // bindless
SamplerState samplers[15];

float3 UniformSampleTriangle(float2 u)
{
    float su0 = sqrt(u.x);
    float b0 = 1.0 - su0;
    float b1 = u.y * su0;
    return float3(b0, b1, 1.0 - b0 - b1);
}

float3 UniformSampleRect(RectLight l, float2 u)
{
    float3 e1 = l.points[1].xyz - l.points[0].xyz;
    float3 e2 = l.points[3].xyz - l.points[0].xyz;
    return l.points[0].xyz + e1 * u.x + e2 * u.y;
}

float3 calcReflection(uint2 pixelIndex)
{
    float4 gbWorldPos = gbWPos[pixelIndex];
    if (gbWorldPos.w == 0.0)
        return 0;
    float3 wpos = gbWPos[pixelIndex].xyz;

    uint rngState = initRNG(pixelIndex, ubo.dimension, ubo.frameNumber);

    float2 rndUV = float2(rand(rngState), rand(rngState));

    RectLight curLight = lights[0];
    float3 pointOnLight = UniformSampleRect(curLight, rndUV);

    float3 L = normalize(pointOnLight - wpos);
    float3 N = normalize(gbNormal[pixelIndex].xyz);

    Ray ray;
    ray.d = float4(L, 0.0);
    const float3 offset = N * 1e-5; // need to add small offset to fix self-collision
    float distToLight = distance(pointOnLight, wpos + offset);
    ray.o = float4(wpos + offset, distToLight - 1e-5);
    Hit hit;
    hit.t = 0.0;

    if ((dot(N, L) > 0.0) && closestHit(ray, hit))
    {
        float2 bcoords = hit.bary;
        InstanceConstants constants = instanceConstants[NonUniformResourceIndex(hit.instId)];
        Material material = materials[hit.materialId];

        /* todo:
         * ? store uv & normals in triangles ? -> bvh ?
         * ? vertex array ? to get uv & normals with instanceConst.vbId
         * ???
         */
        float2 uv0 = float2( 0.383911, 0.079345 );
        float2 uv1 = float2( 0.139917, 0.100708 );
        float2 uv2 = float2( 0.410156, 0.0909424 );
        float2 uvCoord = uv0 * (1 - bcoords.x - bcoords.y) + uv1 * bcoords.x + uv2 * bcoords.y;

        float3 dcol = material.baseColorFactor.rgb;
        if (material.texBaseColor != -1)
        {
            dcol *= textures[NonUniformResourceIndex(material.texBaseColor)].SampleLevel(samplers[NonUniformResourceIndex(material.sampBaseId)], uvCoord, 0).rgb;
        }
        /*
        int index0 = constants.vbId + IndexBuffer[constants.ibId + hit.triangleId + 0];
        int index1 = constants.vbId + IndexBuffer[constants.ibId + hit.triangleId + 1];
        int index2 = constants.vbId + IndexBuffer[constants.ibId + hit.triangleId + 2];

        float3 n0 = Normals[index0].xyz;
        float3 n1 = Normals[index1].xyz;
        float3 n2 = Normals[index2].xyz;

        float3 n = n0 * (1 - bcoords.x - bcoords.y) + n1 * bcoords.x + n2 * bcoords.y;
        */

        return dcol;
    }

    return { 0.f, 0.f, 0.f }; // todo return input[pixelIndex]
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    if (pixelIndex.x >= ubo.dimension.x || pixelIndex.y >= ubo.dimension.y)
    {
        return;
    }

    float3 color = 0.f;

    color = calcReflection(pixelIndex);

    output[pixelIndex] = float4(color, 1.0);
}
