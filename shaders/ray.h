#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#define min glm::min
#define max glm::max
#define inout
#endif

struct Ray
{
    float4 o; // xyz - origin, w - max trace distance
    float4 d;
};

bool intersectRayBox(Ray r, float3 invdir, float3 pmin, float3 pmax, inout float t)
{
    const float3 f = float3(pmax.x - r.o.x, pmax.y - r.o.y, pmax.z - r.o.z) * invdir;
    const float3 n = float3(pmin.x - r.o.x, pmin.y - r.o.y, pmin.z - r.o.z) * invdir;

    const float3 tmax = max(f, n);
    const float3 tmin = min(f, n);

    const float t1 = min(tmax.x, min(tmax.y, tmax.z));
    const float t0 = max(max(tmin.x, max(tmin.y, tmin.z)), 0.0f);

    t = t0;
    return t1 >= t0;
}
