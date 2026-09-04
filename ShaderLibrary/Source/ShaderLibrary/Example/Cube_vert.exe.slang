struct VertexInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

layout(set = 0, binding = 0) ConstantBuffer<TransformBlock> transform;

struct TransformBlock
{
    float4x4 mvp;
};

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    output.position = mul(transform.mvp, float4(input.position, 1.0));
    output.color    = input.color;
    return output;
}