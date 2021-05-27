#pragma once

//  valid range of coordinates [-1; 1]
float3 unpackNormal(uint32_t val)
{
   float3 normal;
   normal.z = ((val & 0xfff00000) >> 20) / 511.99999f * 2.0f - 1.0f;
   normal.y = ((val & 0x000ffc00) >> 10) / 511.99999f * 2.0f - 1.0f;
   normal.x = (val & 0x000003ff) / 511.99999f * 2.0f - 1.0f;

   return normal;
}

//  valid range of coordinates [-10; 10]
float2 unpackUV(uint32_t val)
{
   float2 uv;
   uv.y = ((val & 0xffff0000) >> 16) / 16383.99999f * 20.0f - 10.0f;
   uv.x = (val & 0x0000ffff) / 16383.99999f * 20.0f  - 10.0f;

   return uv;
}

//  valid range of coordinates [-10; 10]
float3 unpackTangent(uint32_t val)
{
   float3 tangent;
   tangent.z = ((val & 0xfff00000) >> 20) / 511.99999f * 20.0f - 10.0f;
   tangent.y = ((val & 0x000ffc00) >> 10) / 511.99999f * 20.0f - 10.0f;
   tangent.x = (val & 0x000003ff) / 511.99999f * 20.0f - 10.0f;

   return tangent;
}
