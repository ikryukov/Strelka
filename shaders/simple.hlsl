struct VertexInput
{
    float3 position : POSITION;
    float3 normal;
    float2 uv;
    uint32_t materialId;
};

struct Material
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float3 emissive;
    float opticalDensity;
    float shininess;
    float3 transparency;
    uint32_t illum;
    uint32_t texAmbientId;
    uint32_t texDiffuseId;
    uint32_t texSpeculaId;
    uint32_t texNormalId;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal;
    float2 uv;
    nointerpolation uint32_t materialId;
    float3 wPos; // new
};

cbuffer ubo
{
    float4x4 modelViewProj;
    float4x4 worldToView;
    float4x4 inverseWorldToView;
    float3 CameraPos; // new
}

Texture2D tex;
SamplerState gSampler;
StructuredBuffer<Material> materials;

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{

    PS_INPUT out;
    out.pos = mul(modelViewProj, float4(vi.position, 1.0f));
    out.uv = vi.uv;
    out.normal = mul((float3x3)inverseWorldToView, vi.normal);
    out.materialId = vi.materialId;
    out.wPos = vi.position;  // ???
    return out;
}


// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PS_INPUT inp) : SV_TARGET
{
   float3 ambient = float3(materials[inp.materialId].ambient.rgb);
   float3 specular = float3(materials[inp.materialId].specular.rgb);
   float3 diffuse = float3(materials[inp.materialId].diffuse.rgb);


   float3 emissive = float3(materials[inp.materialId].emissive.rgb);
   float opticalDensity = float(materials[inp.materialId].opticalDensity);
   float shininess = float(materials[inp.materialId].shininess);
   float3 transparency = float3(materials[inp.materialId].transparency.rgb);
   uint32_t illum = 2;

   uint32_t texAmbientId = 0;
   uint32_t texDiffuseId = 0;
   uint32_t texSpeculaId = 0;
   uint32_t texNormalId = 0;

   //return float4(abs(inp.normal), 1.0);

   /////// new ////////
  // light direction
  // float3 lightDir = normalize(inp.pos - inp.wPos);
  float3 lightDir = float3(10.0f,10.0f,10.0f);
  // per pixel diffuse lighting
  //float diffuseLighting = saturate(dot(inp.normal, -lightDir));
  float diffuseLighting = dot(inp.normal, lightDir);

  //diffuseLighting *= ((length(lightDir) * length(lightDir)) / dot(light.Position - inp.wPos, light.Position - inp.wPos));
  diffuseLighting *= ((length(lightDir) * length(lightDir)) / dot( float3(5.0f,5.0f,5.0f) - inp.wPos,  float3(5.0f,5.0f,5.0f) - inp.wPos));

  float3 h = normalize(normalize(CameraPos - inp.wPos) - lightDir);
  //float specLighting = pow(saturate(dot(h, inp.normal)), 2.0f);
  float specLighting = pow(dot(h, inp.normal), 2.0f);

  return specLighting;

   // return float4(
    //   ambient +
    //    (diffuse  * diffuseLighting * 0.6) + // Use light diffuse vector as intensity multiplier
     //   (specular  * specLighting * 0.5) // Use light specular vector as intensity multiplier
      //  , 1);

}




