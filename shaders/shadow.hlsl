struct VertexInput
{
    float3 position : POSITION;
};

cbuffer ubo
{
    float4x4 lightSpaceMatrix;
    float4x4 modelToWorld;
}

struct PS_INPUT
{
     float4 pos : SV_POSITION;
};

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    //out.pos = mul(mul(lightSpaceMatrix, modelToWorld), float4(vi.position, 1.0)); //?
    out.pos = mul(lightSpaceMatrix, float4(vi.position, 1.0));
    return out;
}

