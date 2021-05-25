struct VertexInput
{
    float3 position : POSITION;
    uint32_t tangent;
    uint32_t normal;
    uint32_t uv;
    uint16_t materialId;
};

struct Material
{
  float4 ambient; // Ka
  float4 diffuse; // Kd
  float4 specular; // Ks
  float4 emissive; // Ke
  float4 transparency; //  d 1 -- прозрачность/непрозрачность
  float opticalDensity; // Ni
  float shininess; // Ns 16 --  блеск материала
  uint32_t illum; // illum 2 -- модель освещения
  int32_t texDiffuseId; // map_diffuse

  int32_t texAmbientId; // map_ambient
  int32_t texSpecularId; // map_specular
  int32_t texNormalId; // map_normal - map_Bump
  uint32_t pad;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 posLightSpace;
    float3 tangent;
    float3 normal;
    float3 wPos;
    float2 uv;
    nointerpolation uint32_t materialId;
};

cbuffer ubo
{
    float4x4 modelToWorld;
    float4x4 modelViewProj;
    float4x4 worldToView;
    float4x4 inverseModelToWorld;
    float4x4 lightSpaceMatrix;
    float4 lightPosition;
    float3 CameraPos;
    float pad;
    uint32_t debugView;
}

Texture2D textures[];
SamplerState gSampler;
StructuredBuffer<Material> materials;
Texture2D shadowMap;
SamplerState shadowSamp;

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

float3 srgb_to_linear(float3 c) {
    return lerp(c / 12.92, pow((c + 0.055) / 1.055, float3(2.4)), step(0.04045, c));
}

float LinearizeDepth(float depth)
{
    float nearPlane = 0.01;
    float farPlane = 50.0;
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

static const float4x4 biasMatrix = float4x4(
  0.5, 0.0, 0.0, 0.5,
  0.0, -0.5, 0.0, 0.5,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0 );

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(vi.position, 1.0f));
    out.posLightSpace = mul(biasMatrix, mul(lightSpaceMatrix, float4(vi.position, 1.0f)));
    out.uv = unpackUV(vi.uv);
    out.normal = mul((float3x3)inverseModelToWorld, (unpackNormal(vi.normal)));
    out.tangent = mul((float3x3)inverseModelToWorld, (unpackTangent(vi.tangent)));
    out.materialId = vi.materialId;
    out.wPos = vi.position;
    return out;
}

float3 diffuseLambert(float3 kD, float3 n, float3 l)
{
    return kD * saturate(dot(l, n));
}

float3 specularPhong(float3 kS, float3 r, float3 v, float shinessFactor)
{
    return kS * pow(saturate(dot(r, v)), shinessFactor);
}

float3 CalcBumpedNormal(PS_INPUT inp, uint32_t texId)
{
    float3 Normal = normalize(inp.normal);
    float3 Tangent = -normalize(inp.tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    float3 Bitangent = cross(Normal, Tangent);

    float3 BumpMapNormal = textures[NonUniformResourceIndex(texId)].Sample(gSampler, inp.uv).xyz;
    BumpMapNormal = BumpMapNormal * 2.0 - 1.0;

    float3x3 TBN = transpose(float3x3(Tangent, Bitangent, Normal));
    float3 NewNormal = normalize(mul(TBN, BumpMapNormal));

    return NewNormal;
}

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

#define INVALID_INDEX -1

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   Material material = materials[NonUniformResourceIndex(inp.materialId)];

   float3 emissive = float3(material.emissive.rgb);
   float opticalDensity = material.opticalDensity;
   float shininess = material.shininess;
   float3 transparency = float3(material.transparency.rgb);
   uint32_t illum = material.illum;

   uint32_t texAmbientId = material.texAmbientId;
   uint32_t texDiffuseId = material.texDiffuseId;
   uint32_t texSpecularId = material.texSpecularId;
   uint32_t texNormalId = material.texNormalId;

   float3 kA = material.ambient.rgb;
   float3 kD = material.diffuse.rgb;
   float3 kS = material.specular.rgb;

   if (texAmbientId != INVALID_INDEX)
   {
      kA *= textures[NonUniformResourceIndex(texAmbientId)].Sample(gSampler, inp.uv).rgb;
   }
   if (texDiffuseId != INVALID_INDEX)
   {
      kD *= textures[NonUniformResourceIndex(texDiffuseId)].Sample(gSampler, inp.uv).rgb;
   }
   if (texSpecularId != INVALID_INDEX)
   {
      kS *= textures[NonUniformResourceIndex(texSpecularId)].Sample(gSampler, inp.uv).rgb;
   }

   float3 N = normalize(inp.normal);
   if (texNormalId != INVALID_INDEX)
   {
      N = CalcBumpedNormal(inp, texNormalId);
   }

   float3 L = normalize(lightPosition.xyz - inp.wPos);
   float3 diffuse = diffuseLambert(kD, L, N);

   float3 R = reflect(-L, N);
   float3 V = normalize(CameraPos - inp.wPos);
   float3 specular = specularPhong(kS, R, V, shininess);

   // Normals
   if (debugView == 1)
   {
      return float4(abs(N), 1.0);
   }
   // Shadow b&w
   if (debugView == 2)
   {
       float shadow = ShadowCalculation(inp.posLightSpace);
       return float4(dot(N, L) * shadow, 1.0);
   }
   // pcf shadow
   if (debugView == 3)
   {
        float shadow = ShadowCalculationPcf(inp.posLightSpace);
        return float4(saturate(kA + diffuse + specular) * shadow, 1.0);
   }
   // poisson shadow
   if (debugView == 4)
   {
        float shadow = ShadowCalculationPoisson(inp.posLightSpace, inp.wPos);
        return float4(saturate(kA + diffuse + specular) * shadow, 1.0);
   }
    // poisson + pcf shadow
    if (debugView == 5)
    {
        float shadow = ShadowCalculationPoissonPCF(inp.posLightSpace, inp.wPos);
        return float4(saturate(kA + diffuse + specular) * shadow, 1.0);
    }

    float shadow = ShadowCalculation(inp.posLightSpace);

    //return float4(saturate(kA + diffuse + specular) * shadow, 1.0);
    return float4(saturate(diffuse) * shadow, 1.0);
}
