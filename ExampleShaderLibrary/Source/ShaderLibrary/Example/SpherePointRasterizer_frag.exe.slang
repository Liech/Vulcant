struct FragmentInput
{
    float4 position : SV_Position;
    float pointSize : PSIZE;
    float4 color : COLOR0;
    float coverage : TEXCOORD0;
};

struct FragmentOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
};

[shader("fragment")]
FragmentOutput main(FragmentInput input)
{
    FragmentOutput output;
    // Scale color and alpha by coverage
    output.color = float4(input.color.rgb, input.color.a * input.coverage);
    // Forward-facing view-space normal
    output.normal = float4(0.5, 0.5, 1.0, 1.0);
    return output;
}
