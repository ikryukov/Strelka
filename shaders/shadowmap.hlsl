struct VertexInput
{
    float3 position : POSITION;
};

struct InstancePushConstants 
{
    float4x4 model;
};
[[vk::push_constant]] ConstantBuffer<InstancePushConstants> pconst;

cbuffer ubo
{
    float4x4 lightSpaceMatrix;
}

struct PS_INPUT
{
     float4 pos : SV_POSITION;
};

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    PS_INPUT out;
    out.pos = mul(lightSpaceMatrix, mul(pconst.model, float4(vi.position, 1.0)));
    return out;
}
