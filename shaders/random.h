#pragma once

uint jenkinsHash(uint x) 
{
    x += x << 10;
    x ^= x >> 6; 
    x += x << 3; 
    x ^= x >> 11; 
    x += x << 15; 
    return x;
}

uint initRNG(uint2 pixel, uint2 resolution, uint frame) 
{
    uint rngState = dot(pixel, uint2(1, resolution.x)) ^ jenkinsHash(frame);
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
