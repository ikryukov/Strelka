#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#endif

#include "helper.h"

struct Light
{
    float3 v0;
    float pad0;
    float3 v1;
    float pad1;
    float3 v2;
    float pad2;
};

struct UniformLight
{
    float4 points[16];
    float4 color;
    float4 normal;
    int type;
    float pad0;
    float pad2;
    float pad3;
};

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
    float3 v00 = {squad.x0, squad.y0, squad.z0};
    float3 v01 = {squad.x0, squad.y1, squad.z0};
    float3 v10 = {squad.x1, squad.y0, squad.z0};
    float3 v11 = {squad.x1, squad.y1, squad.z0};

    // compute normals to edges
    float3 n0 = normalize(cross(v00, v10));
    float3 n1 = normalize(cross(v10, v11));
    float3 n2 = normalize(cross(v11, v01));
    float3 n3 = normalize(cross(v01, v00));

    // compute internal angles (gamma_i)
    float g0 = acos(-dot(n0,n1));
    float g1 = acos(-dot(n1,n2));
    float g2 = acos(-dot(n2,n3));
    float g3 = acos(-dot(n3,n0));

    // compute predefined constants
    squad.b0 = n0.z;
    squad.b1 = n2.z;
    squad.b0sq = squad.b0 * squad.b0;
    squad.k = 2 * PI - g2 - g3;

    // compute solid angle from internal angles
    squad.S = g0 + g1 - squad.k;

    return squad;
}

float3 SphQuadSample(SphQuad squad, float2 uv) {
    float u = uv.x;
    float v = uv.y;

    // 1. compute cu
    float au = u * squad.S + squad.k;
    float fu = (cos(au) * squad.b0 - squad.b1) / sin(au);
    float cu = 1 / sqrt(fu*fu + squad.b0sq) * (fu > 0 ? 1 : -1);
    cu = clamp(cu, -1, 1); // avoid NaNs

    // 2. compute xu
    float xu = -(cu * squad.z0) / sqrt(1 - cu*cu);
    xu = clamp(xu, squad.x0, squad.x1); // avoid Infs

    // 3. compute yv
    float d = sqrt(xu*xu + squad.z0sq);
    float h0 = squad.y0 / sqrt(d*d + squad.y0sq);
    float h1 = squad.y1 / sqrt(d*d + squad.y1sq);
    float hv = h0 + v * (h1 - h0);
    float hv2 = hv*hv;
    float eps = 1e-5;
    float yv = (hv < 1 - eps) ? (hv * d)/sqrt(1 - hv2) : squad.y1;

    // 4. transform (xu, yv, z0) to world coords
    return (squad.o + xu * squad.x + yv*squad.y  + squad.z0*squad.z);
}

float2 concentricSampleDisk(float2 u) {
    // map uniform random numbers to [-1,1]^2
    float2 uOffset = 2.f * u - float2(1, 1);

    // handle degeneracy at the origin
    if (uOffset.x == 0 && uOffset.y == 0)
        return float2(0, 0);

    // apply concentric mapping to point
    float theta, r;
    if (abs(uOffset.x) > abs(uOffset.y)) {
        r = uOffset.x;
        theta = PiOver4 * (uOffset.y / uOffset.x);
    } else {
        r = uOffset.y;
        theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
    }

    return r * float2(cos(theta), sin(theta));
}

float3 UniformSampleLight(in UniformLight l, float2 u)
{
    float3 uniformSample = float3(0.0);

    if (l.type == 0) {
        float3 e1 = l.points[1].xyz - l.points[0].xyz;
        float3 e2 = l.points[3].xyz - l.points[0].xyz;

        uniformSample = l.points[0].xyz + e1 * u.x + e2 * u.y;
    }
    else if (l.type == 1)
    {
        float2 pd = concentricSampleDisk(u);

        float x = l.points[0].x * pd.x;
        float y = l.points[0].x * pd.y;

        uniformSample = l.points[1].xyz + x * l.points[2].xyz + y * l.points[3].xyz;
    }

    return uniformSample;
}

float3 calcLightNormal(in UniformLight l)
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

    return area;
}

float3 sampleLights(inout uint rngState, in Accel accel, in Shading_state_material state, out float3 toLight, out float lightPdf)
{
    uint lightId = (uint) (ubo.numLights * rand(rngState));
    float lightSelectionPdf = 1.0f / (ubo.numLights + 1e-6);
    UniformLight currLight = lights[lightId];
    float3 r = estimateDirectLighting(rngState, accel, currLight, state, toLight, lightPdf);
    lightPdf *= lightSelectionPdf;
    return r / (lightPdf + 1e-5);
}
