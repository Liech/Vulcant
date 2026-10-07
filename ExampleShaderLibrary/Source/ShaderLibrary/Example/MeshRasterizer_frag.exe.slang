import Render.Scene;

[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

public struct MeshVertexOutput
{
    public float4 position   : SV_Position;
    public float4 color      : COLOR0;
    public float3 normalView : TEXCOORD0;
    public float3 posWorld   : TEXCOORD1;
};

public struct RasterizerOutput
{
    public float4 color  : SV_Target0;
    public float4 normal : SV_Target1;
};

[shader("fragment")]
RasterizerOutput main(MeshVertexOutput input)
{
    RasterizerOutput output;

    float3 normalView = normalize(input.normalView);

    // Directional lighting in view space
    float3 lightDirView = normalize(float3(0.3, 0.8, 0.5));
    float diff = max(dot(normalView, lightDirView), 0.0);
    float ambient = 0.25;
    float lighting = ambient + 0.75 * diff;

    output.color = float4(input.color.rgb * lighting, 1.0);
    output.normal = float4(normalView * 0.5 + 0.5, 1.0);

    return output;
}
