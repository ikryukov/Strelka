struct VertexInput
{
    float3 position : POSITION;
};

struct InstanceConstants
{
    float4x4 model;
    float4x4 normalMatrix;
    int32_t materialId;
    int32_t pad0;
    int32_t pad1;
    int32_t pad2;
};

struct InstancePushConstants 
{
    int32_t instanceId;
};
[[vk::push_constant]] ConstantBuffer<InstancePushConstants> pconst;

cbuffer ubo
{
    float4x4 lightSpaceMatrix;
}

StructuredBuffer<InstanceConstants> instanceConstants;

struct PS_INPUT
{
     float4 pos : SV_POSITION;
};

[shader("vertex")]
PS_INPUT vertexMain(VertexInput vi)
{
    InstanceConstants constants = instanceConstants[NonUniformResourceIndex(pconst.instanceId)];
    PS_INPUT out;
    out.pos = mul(lightSpaceMatrix, mul(constants.model, float4(vi.position, 1.0)));
    return out;
}
