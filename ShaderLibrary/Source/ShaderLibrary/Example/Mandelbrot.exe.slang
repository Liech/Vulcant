[shader("compute")]
[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint width, height;
    resultImage.GetDimensions(width, height);
    uint2 dims = uint2(width, height);
    uint2 pixel_coords = dispatchThreadID.xy;

    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y)
        return;

    float2 uv = float2(pixel_coords) / float2(dims);

    float n = 0.0;
    float2 c = float2(-0.445, 0.0) + (uv - 0.5) * (2.0 + 1.7 * 0.2);
    float2 z = float2(0.0);
    const int M = 128;

    for (int i = 0; i < M; i++)
    {
        z = float2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 4.0) 
            break;
        n++;
    }

    float t = n / float(M);
    float3 d = float3(0.3, 0.3, 0.5);
    float3 e = float3(-0.2, -0.3, -0.5);
    float3 f = float3(2.1, 2.0, 3.0);
    float3 g = float3(0.0, 0.1, 0.0);

    float4 color = float4(d + e * cos(6.28318 * (f * t + g)), 1.0);

    resultImage[pixel_coords] = color;
}

[format("rgba32f")]
RWTexture2D<float4> resultImage;