import Render.Scene;

// Set 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

public struct MeshVertexInput
{
    public float3 position : POSITION;
    public float3 normal   : NORMAL;
    public float3 color    : COLOR;
};

public struct MeshVertexOutput
{
    public float4 position   : SV_Position;
    public float4 color      : COLOR0;
    public float3 normalView : TEXCOORD0;
    public float3 posWorld   : TEXCOORD1;
};

[shader("vertex")]
MeshVertexOutput main(MeshVertexInput input)
{
    MeshVertexOutput output;

    float4 worldPos = float4(input.position, 1.0);
    float4 viewPos  = mul(scene.viewMatrix, worldPos);

    output.position   = mul(scene.projectionMatrix, viewPos);
    output.color      = float4(input.color, 1.0);
    output.normalView = normalize(mul((float3x3)scene.viewMatrix, input.normal));
    output.posWorld   = input.position;

    return output;
}
