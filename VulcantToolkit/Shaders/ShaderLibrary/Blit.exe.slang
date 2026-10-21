[[vk::binding(0, 0)]]
RWTexture2D<float4> screen_image;

[[vk::binding(0, 1)]]
Sampler2D input_color_tex;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint2 coords = dispatchThreadID.xy;
    uint2 size;
    screen_image.GetDimensions(size.x, size.y);

    if (coords.x >= size.x || coords.y >= size.y)
        return;

    float2 uv = (float2(coords) + 0.5) / float2(size);
    float2 input_uv = float2(uv.x, 1.0f - uv.y);
    float4 color      = input_color_tex.Sample(input_uv);
    float4 godot_color = screen_image[coords];

    if (color.a > 0.0f)
    {
        float3 final_color = lerp(godot_color.rgb, color.rgb, color.a);
        screen_image[coords] = float4(final_color, 1.0f);
    }
}