module BillboardRasterizer;

import Utility.HalfPacking;

public struct SphereData
{
    public float3 center;
    public uint radiusHalfAndColor; // bits 0..15 = FP16 radius, bits 16..31 = RGBA4444 color

    public float getRadius()
    {
        return unpackHalf(radiusHalfAndColor & 0xFFFFu);
    }

    public float4 getColor()
    {
        uint color16 = radiusHalfAndColor >> 16u;
        return float4(
            float(color16 & 0x0Fu) / 15.0,
            float((color16 >> 4u) & 0x0Fu) / 15.0,
            float((color16 >> 8u) & 0x0Fu) / 15.0,
            float((color16 >> 12u) & 0x0Fu) / 15.0
        );
    }
};

public static const float2 quadOffsets[6] = {
    float2(-1.0, -1.0),
    float2( 1.0, -1.0),
    float2( 1.0,  1.0),
    float2(-1.0, -1.0),
    float2( 1.0,  1.0),
    float2(-1.0,  1.0)
};

public struct RasterizerOutput
{
    public float4 color : SV_Target0;
    public float4 normal : SV_Target1;
    public float depth : SV_Depth;
};
