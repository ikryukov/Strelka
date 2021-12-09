#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#define min glm::min
#define max glm::max
#define inout
#define out
#endif

struct Ray
{
    float4 o; // xyz - origin, w - max trace distance
    float4 d;
};

// https://interplayoflight.wordpress.com/2018/07/04/hybrid-raytraced-shadows-and-reflections/
#ifdef __cplusplus
bool intersectRayBox(Ray& r, float3 invdir, float3 pmin, float3 pmax, float& t)
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
#else
bool intersectRayBox(Ray r, float3 invdir, float3 pmin, float3 pmax, out float t)
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
//{
//    float3 tMin = (pmin - r.o.xyz) * invdir;
//    float3 tMax = (pmax - r.o.xyz) * invdir;
//    float3 t1 = min(tMin, tMax);
//    float3 t2 = max(tMin, tMax);
//    float tNear = max(max(t1.x, t1.y), t1.z);
//    float tFar = min(min(t2.x, t2.y), t2.z);
//    t = tNear;
//    return tNear <= tFar;
//}
#endif

#ifdef __cplusplus
#    undef float4
#    undef float3
#    undef uint
#    undef max
#    undef min
#    undef out
#    undef inout
#endif
