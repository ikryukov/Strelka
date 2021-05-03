struct VertexInput
{
    float3 position : POSITION;
};

cbuffer ubo //?
{
    float4x4 lightSpaceMatrix;
    float4x4 modelToWorld;
    float4x4 modelViewProj;
    float4 lightPosition;
}

struct PS_INPUT
{
     float4 pos : SV_POSITION; //?
};

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(mul(lightSpaceMatrix, modelViewProj), float4(vi.position, 1.0f));

    return out;
}

