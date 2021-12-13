#pragma once

#ifdef __cplusplus
#define float4 glm::float4
#define float3 glm::float3
#define float2 glm::float2
#define int2 glm::int2
#define min glm::min
#define max glm::max
#define inout
#endif

#ifndef __cplusplus
uint jenkinsHash(uint x) 
{
    x += x << 10;
    x ^= x >> 6; 
    x += x << 3; 
    x ^= x >> 11; 
    x += x << 15; 
    return x;
}

uint initRNG(int2 pixel, int2 resolution, uint frame)
{
    uint t = dot(float2(pixel), float2(1, resolution.x));
    uint rngState = t ^ jenkinsHash(frame);
    return jenkinsHash(rngState); 
}

float uintToFloat(uint x) 
{
    return asfloat(0x3f800000 | (x >> 9)) - 1.f;
}

uint xorshift(inout uint rngState) 
{
    rngState ^= rngState << 13; 
    rngState ^= rngState >> 17; 
    rngState ^= rngState << 5; 
    return rngState;
}

float rand(inout uint rngState) 
{
    return uintToFloat(xorshift(rngState));
}
#endif

#ifdef __cplusplus
#    undef float4
#    undef float3
#    undef float2
#    undef uint
#    undef int2
#    undef max
#    undef min
#endif
