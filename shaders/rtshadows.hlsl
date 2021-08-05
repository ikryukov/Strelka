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
    float3 v;
    int pad;
};

struct Light
{
    float3 pos;
    float pad;
};

struct Ray
{
    float4 o;
    float4 d;
};

Texture2D<float4> gbWPos;
Texture2D<float4> gbNormal;

StructuredBuffer<BVHNode> bvh;
StructuredBuffer<Light> lights;

RWTexture2D<float> output;

#define INVALID_INDEX 0xFFFFFFFF
#define PI 3.1415926535897

bool intersectRayBox(Ray r, float3 invdir, float3 pmin, float3 pmax)
{
    const float3 f = (pmax.xyz - r.o.xyz) * invdir;
    const float3 n = (pmin.xyz - r.o.xyz) * invdir;

    const float3 tmax = max(f, n);
    const float3 tmin = min(f, n);

    const float t1 = min(tmax.x, min(tmax.y, tmax.z));
    const float t0 = max(max(tmin.x, max(tmin.y, tmin.z)), 0.0f);

    return t1 >= t0;
}

bool intersectRayTri(Ray r, float3 v0, float3 e0, float3 e1)
{
    const float3 s1 = cross(r.d.xyz, e1);
    const float  invd = 1.0 / (dot(s1, e0));
    const float3 d = r.o.xyz - v0;
    const float  b1 = dot(d, s1) * invd;
    const float3 s2 = cross(d, e0);
    const float  b2 = dot(r.d.xyz, s2) * invd;
    const float temp = dot(e1, s2) * invd;

    if (b1 < 0.0 || b1 > 1.0 || b2 < 0.0 || b1 + b2 > 1.0 || temp < 0.0 || temp > r.o.w)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool anyHit(Ray ray)
{
    const float3 invdir = 1.0 / ray.d.xyz;
    uint32_t nodeIndex = 0;
    while (nodeIndex != INVALID_INDEX)
    {
        BVHNode node = bvh[nodeIndex];
        uint32_t primitiveIndex = node.instId;
        if (primitiveIndex != INVALID_INDEX) // leaf
        {
            if (intersectRayTri(ray, node.v, node.minBounds, node.maxBounds))
            {
                return true;
            }
        }
        else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds))
        {
            ++nodeIndex;
            continue;
        }
        nodeIndex = node.nodeOffset;
    }
    return false;
}

float calcShadow(uint2 pixelIndex)
{
    float3 wpos = gbWPos[pixelIndex].xyz;
    float3 lightPosition = lights[0].pos.xyz;
    float3 L = normalize(lightPosition.xyz - wpos);
    float3 N = gbNormal[pixelIndex].xyz;
    
    Ray ray;

    ray.d = float4(L, 0.0);
    ray.o = float4(wpos, 1e9);

    if (anyHit(ray))
    {
        return 0.1;
    }
    return 1.0;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void computeMain(uint2 pixelIndex : SV_DispatchThreadID)
{
    output[pixelIndex] = calcShadow(pixelIndex);
}
