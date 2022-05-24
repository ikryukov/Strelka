#pragma once

#ifdef __cplusplus
#    define float4 glm::float4
#    define float3 glm::float3
#endif

#include "helper.h"

struct UniformLight
{
    float4 points[4];
    float4 color;
    float4 normal;
    int type;
    float pad0;
    float pad2;
    float pad3;
};

struct LightSampleData
{
    float3 pointOnLight;
    float pdf;

    float3 normal;
    float area;

    float3 L;
    float distToLight;
};

// https://backend.orbit.dtu.dk/ws/portalfiles/portal/126824972/onb_frisvad_jgt2012_v2.pdf
void frisvad(const float3 n, in out float3 b1, in out float3 b2)
{
    if (n.z < -0.9999999f) // Handle the singularity
    {
        b1 = float3(0.0f, -1.0f, 0.0f);
        b2 = float3(-1.0f, 0.0f, 0.0f);
        return;
    }
    const float a = 1.0f / (1.0f + n.z);
    const float b = -n.x * n.y * a;
    b1 = float3(1.0f - n.x * n.x * a, b, -n.x);
    b2 = float3(b, 1.0f - n.y * n.y * a, -n.y);
}

float3 sphericalToCartesian(float theta, float phi)
{
    float3 coord;
    coord.x = 5.0f * sin(phi) * sin(theta);
    coord.z = -5.0f * sin(phi) * cos(theta);
    coord.y = 5.0f * cos(phi);

    return coord;
}

// https://schuttejoe.github.io/post/arealightsampling/
float3 sampleSphereSolidAngle(in UniformLight l, float3 surfaceNormal, float3 hitPoint, float2 u)
{
    float3 c = l.points[1].xyz;
    float r = l.points[0].x;

    float3 o = hitPoint;

    float3 w = c - o;
    float distanceToCenter = length(w);
    w = w * (1.0f / distanceToCenter);

    float q = sqrt(1.0f - (r / distanceToCenter) * (r / distanceToCenter));

    float3 n = surfaceNormal;
    float3 v, u;
    frisvad(w, v, u); // MakeOrthonormalCoordinateSystem

    float3x3 toWorld = float3x3(u, w, v);

    float r0 = u.x;
    float r1 = u.y;

    float theta = acos(1 - r0 + r0 * q);
    float phi = 2 * PI * r1;

    float3 local = sphericalToCartesian(theta, phi);

    float3 nwp = mul(local, toWorld);

    return nwp;
}

struct SphQuad
{
    float3 o, x, y, z;
    float z0, z0sq;
    float x0, y0, y0sq; // rectangle coords in ’R’
    float x1, y1, y1sq;
    float b0, b1, b0sq, k;
    float S;
};

// Precomputation of constants for the spherical rectangle Q.
SphQuad init(UniformLight l, float3 o)
{
    SphQuad squad;

    float3 ex = l.points[1].xyz - l.points[0].xyz;
    float3 ey = l.points[3].xyz - l.points[0].xyz;

    float3 s = l.points[0].xyz;

    float exl = length(ex);
    float eyl = length(ey);

    squad.o = o;
    squad.x = ex / exl;
    squad.y = ey / eyl;
    squad.z = cross(squad.x, squad.y);

    // compute rectangle coords in local reference system
    float3 d = s - o;
    squad.z0 = dot(d, squad.z);

    // flip ’z’ to make it point against ’Q’
    if (squad.z0 > 0)
    {
        squad.z *= -1;
        squad.z0 *= -1;
    }

    squad.z0sq = squad.z0 * squad.z0;
    squad.x0 = dot(d, squad.x);
    squad.y0 = dot(d, squad.y);
    squad.x1 = squad.x0 + exl;
    squad.y1 = squad.y0 + eyl;
    squad.y0sq = squad.y0 * squad.y0;
    squad.y1sq = squad.y1 * squad.y1;

    // create vectors to four vertices
    float3 v00 = { squad.x0, squad.y0, squad.z0 };
    float3 v01 = { squad.x0, squad.y1, squad.z0 };
    float3 v10 = { squad.x1, squad.y0, squad.z0 };
    float3 v11 = { squad.x1, squad.y1, squad.z0 };

    // compute normals to edges
    float3 n0 = normalize(cross(v00, v10));
    float3 n1 = normalize(cross(v10, v11));
    float3 n2 = normalize(cross(v11, v01));
    float3 n3 = normalize(cross(v01, v00));

    // compute internal angles (gamma_i)
    float g0 = acos(-dot(n0, n1));
    float g1 = acos(-dot(n1, n2));
    float g2 = acos(-dot(n2, n3));
    float g3 = acos(-dot(n3, n0));

    // compute predefined constants
    squad.b0 = n0.z;
    squad.b1 = n2.z;
    squad.b0sq = squad.b0 * squad.b0;
    squad.k = 2 * PI - g2 - g3;

    // compute solid angle from internal angles
    squad.S = g0 + g1 - squad.k;

    return squad;
}

float3 SphQuadSample(SphQuad squad, float2 uv)
{
    float u = uv.x;
    float v = uv.y;

    // 1. compute cu
    float au = u * squad.S + squad.k;
    float fu = (cos(au) * squad.b0 - squad.b1) / sin(au);
    float cu = 1 / sqrt(fu * fu + squad.b0sq) * (fu > 0 ? 1 : -1);
    cu = clamp(cu, -1, 1); // avoid NaNs

    // 2. compute xu
    float xu = -(cu * squad.z0) / sqrt(1 - cu * cu);
    xu = clamp(xu, squad.x0, squad.x1); // avoid Infs

    // 3. compute yv
    float d = sqrt(xu * xu + squad.z0sq);
    float h0 = squad.y0 / sqrt(d * d + squad.y0sq);
    float h1 = squad.y1 / sqrt(d * d + squad.y1sq);
    float hv = h0 + v * (h1 - h0);
    float hv2 = hv * hv;
    float eps = 1e-5;
    float yv = (hv < 1 - eps) ? (hv * d) / sqrt(1 - hv2) : squad.y1;

    // 4. transform (xu, yv, z0) to world coords
    return (squad.o + xu * squad.x + yv * squad.y + squad.z0 * squad.z);
}

float2 concentricSampleDisk(float2 u)
{
    // map uniform random numbers to [-1,1]^2
    float2 uOffset = 2.f * u - float2(1, 1);

    // handle degeneracy at the origin
    if (uOffset.x == 0 && uOffset.y == 0)
        return float2(0, 0);

    // apply concentric mapping to point
    float theta, r;
    if (abs(uOffset.x) > abs(uOffset.y))
    {
        r = uOffset.x;
        theta = PiOver4 * (uOffset.y / uOffset.x);
    }
    else
    {
        r = uOffset.y;
        theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
    }

    return r * float2(cos(theta), sin(theta));
}

float3 UniformSampleSphere(float2 u)
{
    float z = 1 - 2 * u[0];
    float r = sqrt(max((float)0, (float)1. - z * z));
    float phi = 2 * PI * u[1];

    return float3(r * cos(phi), r * sin(phi), z);
}

float3 calcLightNormal(in UniformLight l, float3 hitPoint)
{
    float3 norm = float3(0.0);

    if (l.type == 0)
    {
        float3 e1 = l.points[1].xyz - l.points[0].xyz;
        float3 e2 = l.points[3].xyz - l.points[0].xyz;

        norm = -normalize(cross(e1, e2));
    }
    else if (l.type == 1)
    {
        norm = l.normal.xyz;
    }
    else if (l.type == 2)
    {
        norm = normalize(hitPoint - l.points[1].xyz);
    }

    return norm;
}

float calcLightArea(in UniformLight l)
{
    float area = 0.0f;

    if (l.type == 0) // rectangle area
    {
        float3 e1 = l.points[1].xyz - l.points[0].xyz;
        float3 e2 = l.points[3].xyz - l.points[0].xyz;
        area = length(cross(e1, e2));
    }
    else if (l.type == 1) // disc area
    {
        area = PI * l.points[0].x * l.points[0].x; // pi * radius^2
    }
    else if (l.type == 2) // sphere area
    {
        area = 4 * PI * l.points[0].x * l.points[0].x; // 4 * pi * radius^2
    }
    return area;
}

void fillLightData(in UniformLight l, float3 hitPoint, in out LightSampleData lightSampleData)
{
    lightSampleData.area = calcLightArea(l);
    lightSampleData.normal = calcLightNormal(l, hitPoint);
    const float3 toLight = lightSampleData.pointOnLight - hitPoint;
    const float lenToLight = length(toLight);
    lightSampleData.L = toLight / lenToLight;
    lightSampleData.distToLight = lenToLight;
}

LightSampleData SampleDomeLight(in UniformLight l, float3 surfaceNormal, float3 hitPoint, float2 u)
{
    LightSampleData lightSampleData;

    lightSampleData.pointOnLight = l.points[0].xyz;
    lightSampleData.L = SampleRayInHemisphere(hitPoint, u);
    lightSampleData.normal = normalize(-surfaceNormal);
    lightSampleData.distToLight = 1000000.f;
    lightSampleData.pdf = 1.0f;

    return lightSampleData;
}

LightSampleData SampleRectLight(in UniformLight l, float2 u, float3 hitPoint)
{
    LightSampleData lightSampleData;
    // uniform sampling
//    float3 e1 = l.points[1].xyz - l.points[0].xyz;
//    float3 e2 = l.points[3].xyz - l.points[0].xyz;
//
//    lightSampleData.pointOnLight = l.points[0].xyz + e1 * u.x + e2 * u.y;

    SphQuad quad = init(l, hitPoint);
    lightSampleData.pointOnLight = SphQuadSample(quad, u);

    fillLightData(l, hitPoint, lightSampleData);

    lightSampleData.pdf =  lightSampleData.distToLight * lightSampleData.distToLight / (-dot(lightSampleData.L,  lightSampleData.normal) *  lightSampleData.area);

    return lightSampleData;
}

LightSampleData SampleDiscLight(in UniformLight l, float2 u, float3 hitPoint)
{
    LightSampleData lightSampleData;
    // uniform sampling
    float2 pd = concentricSampleDisk(u);

    float x = l.points[0].x * pd.x;
    float y = l.points[0].x * pd.y;

    lightSampleData.pointOnLight = l.points[1].xyz + x * l.points[2].xyz + y * l.points[3].xyz;

    fillLightData(l, hitPoint, lightSampleData);

    lightSampleData.pdf = lightSampleData.distToLight * lightSampleData.distToLight / (-dot(lightSampleData.L,  lightSampleData.normal) *  lightSampleData.area);

    return lightSampleData;
}

LightSampleData SampleSphereLight(in UniformLight l, float3 surfaceNormal, float3 hitPoint, float2 u)
{
    LightSampleData lightSampleData;
    // uniform sampling
    // lightSampleData.pointOnLight = l.points[1].xyz + l.points[0].x * UniformSampleSphere(u);

    lightSampleData.pointOnLight = sampleSphereSolidAngle(l, surfaceNormal, hitPoint, u);

    fillLightData(l, hitPoint, lightSampleData);

    float3 c = l.points[1].xyz;
    float distanceToCenter = length(c - hitPoint);

    float sinThetaMax2 = l.points[0].x * l.points[0].x / (distanceToCenter * distanceToCenter);
    float cosThetaMax = sqrt(max((float)0, 1 - sinThetaMax2));

    lightSampleData.pdf = 1 / (2 * PI * (1 - cosThetaMax));

    return lightSampleData;
}
