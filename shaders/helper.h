#pragma once

float3 interpolateAttrib(float3 attr1, float3 attr2, float3 attr3, float2 bary)
{
    return attr1 * (1 - bary.x - bary.y) + attr2 * bary.x + attr3 * bary.y;
}

float2 interpolateAttrib(float2 attr1, float2 attr2, float2 attr3, float2 bary)
{
    return attr1 * (1 - bary.x - bary.y) + attr2 * bary.x + attr3 * bary.y;
}
