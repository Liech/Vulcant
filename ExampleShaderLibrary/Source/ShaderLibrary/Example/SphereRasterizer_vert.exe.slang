import Render.Scene;

struct SphereData
{
    float3 center;
    uint radiusHalfAndColor; // bits 0..15 = FP16 radius, bits 16..31 = RGBA4444 color
};

struct SphereVertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 sphereCenterView : TEXCOORD1;
    float sphereRadius : PSIZE;
    float4 color : COLOR0;
    float4 projParams : TEXCOORD2;
};

// Set 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

// Set 1: Spheres
[[vk::binding(0, 1)]] StructuredBuffer<SphereData> gSpheres;

static const float2 quadOffsets[6] = {
    float2(-1.0, -1.0),
    float2( 1.0, -1.0),
    float2( 1.0,  1.0),
    float2(-1.0, -1.0),
    float2( 1.0,  1.0),
    float2(-1.0,  1.0)
};

float unpackHalf(uint val16)
{
    uint sign = (val16 & 0x8000u) << 16u;
    uint exp  = (val16 & 0x7C00u) >> 10u;
    uint mant = (val16 & 0x03FFu);

    if (exp == 0u)
    {
        if (mant == 0u)
        {
            return asfloat(sign);
        }
        while ((mant & 0x0400u) == 0u)
        {
            mant <<= 1u;
            exp--;
        }
        exp++;
        mant &= ~0x0400u;
    }
    else if (exp == 31u)
    {
        exp = 255u;
    }
    else
    {
        exp = exp + (127u - 15u);
    }

    uint res = sign | (exp << 23u) | (mant << 13u);
    return asfloat(res);
}

[shader("vertex")]
SphereVertexOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    SphereVertexOutput output;

    uint sphereID = instanceID;
    uint cornerID = vertexID % 6u;
    float2 quadPos = quadOffsets[cornerID];

    SphereData sphere = gSpheres[sphereID];
    float radius = unpackHalf(sphere.radiusHalfAndColor & 0xFFFFu);
    float r = abs(radius);

    // Center in view space
    float4 centerView = mul(scene.viewMatrix, float4(sphere.center, 1.0));

    // Billboard quad expansion in view space (camera-facing)
    float3 cornerView = centerView.xyz + float3(quadPos * r, 0.0);

    output.position = mul(scene.projectionMatrix, float4(cornerView, 1.0));
    output.uv = quadPos;
    output.sphereCenterView = centerView.xyz;
    output.sphereRadius = radius;

    // Pass projection parameters for exact depth calculation in fragment shader without needing descriptor sets
    output.projParams = float4(scene.projectionMatrix[2][2], scene.projectionMatrix[2][3], scene.projectionMatrix[3][2], scene.projectionMatrix[3][3]);

    // Unpack RGBA4444 color
    uint color16 = sphere.radiusHalfAndColor >> 16u;
    output.color = float4(
        float(color16 & 0x0Fu) / 15.0,
        float((color16 >> 4u) & 0x0Fu) / 15.0,
        float((color16 >> 8u) & 0x0Fu) / 15.0,
        float((color16 >> 12u) & 0x0Fu) / 15.0
    );

    return output;
}
