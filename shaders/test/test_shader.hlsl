// shaders.slang

//
// This file provides a simple vertex and fragment shader that can be compiled
// using Slang. This code should also be valid as HLSL, and thus it does not
// use any of the new language features supported by Slang.

// Uniform data to be passed from application -> shader.
cbuffer Uniforms
{
    float4x4 modelViewProjection;
}
Texture2D<float> shadow;

// Per-vertex attributes to be assembled from bound vertex buffers.
struct AssembledVertex
{
    float3	position : POSITION;
    float3	color    : COLOR;
};

// Output of the vertex shader, and input to the fragment shader.
struct CoarseVertex
{
    float3 color;
};

// Output of the fragment shader
struct Fragment
{
    float4 color;
};

// Vertex  Shader

struct VertexStageOutput
{
    CoarseVertex    coarseVertex    : CoarseVertex;
    float4          sv_position     : SV_Position;
};


  struct structtype0 {
  float3 m_0;
  float3 m_1;
  float3 m_2;
  };

  structtype0 constr_structtype0(float3 m_0, float3 m_1, float3 m_2)
  {
  structtype0 res;
  res.m_0 = m_0;
  res.m_1 = m_1;
  res.m_2 = m_2;
  return res;
  }

[shader("vertex")]
VertexStageOutput vertexMain(
    AssembledVertex assembledVertex)
{
    VertexStageOutput output;

    float3 position = assembledVertex.position;
    float3 color    = assembledVertex.color;

    output.coarseVertex.color = color;
    output.sv_position = mul(modelViewProjection, float4(position, 1.0));

    return output;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(
    CoarseVertex coarseVertex : CoarseVertex) : SV_Target
{
    float3 color = coarseVertex.color;

    return float4(color, 1.0);
}
