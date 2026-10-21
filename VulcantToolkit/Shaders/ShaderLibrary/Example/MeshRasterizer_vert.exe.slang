import Render.Scene;

[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

public struct MeshParams
{
    public float4 wireframeColor;
    public int    wireframeMode;
    public float3 padding;
};

[[vk::binding(1, 0)]] ConstantBuffer<MeshParams> meshParams;

public struct MeshVertexInput
{
    public float3 position : POSITION;
    public float3 normal   : NORMAL;
    public float3 color    : COLOR;
};

public struct MeshVertexOutput
{
    public float4 position    : SV_Position;
    public float4 color       : COLOR0;
    public float3 normalView  : TEXCOORD0;
    public float3 posWorld    : TEXCOORD1;
    public float3 barycentric : COLOR1;
};

[shader("vertex")]
MeshVertexOutput main(MeshVertexInput input, uint vertexID : SV_VertexID)
{
    MeshVertexOutput output;

    float4 worldPos = float4(input.position, 1.0);
    float4 viewPos  = mul(scene.viewMatrix, worldPos);

    output.position   = mul(scene.projectionMatrix, viewPos);
    if (meshParams.wireframeMode < 0)
    {
        output.position.w += 0.00001 * meshParams.wireframeColor.x;
    }
    output.color      = float4(input.color, 1.0);
    output.normalView = normalize(mul((float3x3)scene.viewMatrix, input.normal));
    output.posWorld   = input.position;

    uint corner = vertexID % 3u;
    output.barycentric = (corner == 0u) ? float3(1.0, 0.0, 0.0) : ((corner == 1u) ? float3(0.0, 1.0, 0.0) : float3(0.0, 0.0, 1.0));

    return output;
}
