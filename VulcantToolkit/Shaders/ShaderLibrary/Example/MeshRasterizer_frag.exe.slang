import Render.Scene;

[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

public struct MeshParams
{
    public float4 wireframeColor;
    public int    wireframeMode;
    public float3 padding;
};

[[vk::binding(1, 0)]] ConstantBuffer<MeshParams> meshParams;

public struct MeshVertexOutput
{
    public float4 position    : SV_Position;
    public float4 color       : COLOR0;
    public float3 normalView  : TEXCOORD0;
    public float3 posWorld    : TEXCOORD1;
    public float3 barycentric : COLOR1;
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

    float3 lightDirView = normalize(float3(0.3, 0.8, 0.5));
    float diff = max(dot(normalView, lightDirView), 0.0);
    float ambient = 0.25;
    float lighting = ambient + 0.75 * diff;

    float4 surfaceColor = float4(input.color.rgb * lighting, 1.0);

    if (meshParams.wireframeMode == 1 || meshParams.wireframeMode == 2)
    {
        float3 bary = input.barycentric;
        float3 deltas = fwidth(bary);
        float lineWidth = meshParams.wireframeColor.w > 0.0 ? meshParams.wireframeColor.w : 1.5;
        float3 smoothing = max(deltas * lineWidth, float3(1e-4));
        float3 edge = smoothstep(float3(0.0), smoothing, bary);
        float minEdge = min(min(edge.x, edge.y), edge.z);
        float wireFactor = 1.0 - minEdge;

        if (meshParams.wireframeMode == 2)
        {
            if (wireFactor < 0.1)
            {
                discard;
            }
            output.color = float4(meshParams.wireframeColor.rgb, 1.0);
        }
        else
        {
            output.color = float4(lerp(surfaceColor.rgb, meshParams.wireframeColor.rgb, wireFactor), 1.0);
        }
    }
    else
    {
        output.color = surfaceColor;
    }

    output.normal = float4(normalView * 0.5 + 0.5, 1.0);

    return output;
}
