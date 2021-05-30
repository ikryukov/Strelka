#pragma once

float ShadowCalculation(float4 lightCoord)
{
    const float bias = 0.005;
    float shadow = 1.0;
    float3 projCoord = lightCoord.xyz / lightCoord.w;
    if (abs(projCoord.z) < 1.0)
    {
        float2 coord = float2(projCoord.x, projCoord.y);
        float distFromLight = shadowMap.Sample(shadowSamp, coord).r;
        if (distFromLight < projCoord.z)
        {
            shadow = 0.1;
        }
    }

    return shadow;
}

float ShadowCalculationPcf(float4 lightCoord)
{
    const float bias = 0.005;
    float shadow = 0.1;
    float3 projCoord = lightCoord.xyz / lightCoord.w;
    if (abs(projCoord.z) < 1.0)
    {
      float2 coord = float2(projCoord.x, projCoord.y);
      // pcf
      float2 texelSize = 1.0 / float2(1024, 1024);
        for (int x = -1; x <= 1; ++x)
        {
            for(int y = -1; y <= 1; ++y)
            {
                float pcfDepth = shadowMap.Sample(shadowSamp, coord + float2(x, y) * texelSize).r;
                shadow += projCoord.z > pcfDepth ? 0.1 : 1.0;
            }
        }
        shadow /= 9.0;
    }

    return shadow;
}

// Returns a random number based on a float3 and an int.
float random(float3 seed, int i){
	float4 seed4 = float4(seed,i);
	float dot_product = dot(seed4, float4(12.9898, 78.233, 45.164, 94.673));

	return frac(sin(dot_product) * 43758.5453);
}

float ShadowCalculationPoisson(float4 lightCoord, float3 wPos)
{
    float2 poissonDisk[16] = {
        float2(-0.94201624, -0.39906216),  float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
        float2(-0.91588581, 0.45771432),   float2(-0.81544232, -0.87912464),
        float2(-0.38277543, 0.27676845),   float2(0.97484398, 0.75648379),
        float2(0.44323325, -0.97511554),   float2(0.53742981, -0.47373420),
        float2(-0.26496911, -0.41893023),  float2(0.79197514, 0.19090188),
        float2(-0.24188840, 0.99706507),   float2(-0.81409955, 0.91437590),
        float2(0.19984126, 0.78641367),    float2(0.14383161, -0.14100790)};

    float shadow = 1.0;
    float3 projCoord = lightCoord.xyz / lightCoord.w;
    if (abs(projCoord.z) < 1.0)
    {
        float2 coord = float2(projCoord.x, projCoord.y);
        // poisson
        for (int i = 0; i < 4; i++)
        {
            int index = int(16.0 * random(floor(wPos.xyz * 1000.0), i)) % 16;
            if (shadowMap.Sample(shadowSamp, coord + poissonDisk[index] / 700).r < projCoord.z)
            {
                shadow -= 0.2;
            }
        }
    }

    return shadow;
}

float ShadowCalculationPoissonPCF(float4 lightCoord, float3 wPos)
{
  float2 poissonDisk[16] = {
      float2(-0.94201624, -0.39906216),  float2(0.94558609, -0.76890725),
      float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
      float2(-0.91588581, 0.45771432),   float2(-0.81544232, -0.87912464),
      float2(-0.38277543, 0.27676845),   float2(0.97484398, 0.75648379),
      float2(0.44323325, -0.97511554),   float2(0.53742981, -0.47373420),
      float2(-0.26496911, -0.41893023),  float2(0.79197514, 0.19090188),
      float2(-0.24188840, 0.99706507),   float2(-0.81409955, 0.91437590),
      float2(0.19984126, 0.78641367),    float2(0.14383161, -0.14100790)};

  float shadow = 1.0;
  float3 projCoord = lightCoord.xyz / lightCoord.w;
  float2 coord = float2(projCoord.x, projCoord.y);

  if (abs(projCoord.z) < 1.0)
  {
      float2 texelSize = 1.0 / float2(1024, 1024);
      // pcf
      for (int x = -1; x <= 1; ++x)
      {
          for(int y = -1; y <= 1; ++y)
          {
              float pcfDepth = shadowMap.Sample(shadowSamp, coord + float2(x, y) * texelSize).r;
              if (projCoord.z > pcfDepth)
              {
                  shadow += 0.1;
              }
              else
              {
                  shadow += 0.8;
                  // poisson
                  for (int i = 0; i < 4; i++)
                  {
                      int index = int(16.0 * random(floor(wPos.xyz * 1000.0), i)) % 16;
                      if (shadowMap.Sample(shadowSamp, coord + poissonDisk[index] / 700).r < projCoord.z)
                      {
                          shadow -= 0.2;
                      }
                  }
              }
          }
      }
      shadow /= 9.0;
  }

  return shadow;
}
