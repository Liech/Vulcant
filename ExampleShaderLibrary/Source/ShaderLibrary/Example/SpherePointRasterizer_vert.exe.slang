import Render.Scene;
import Utility.HalfPacking;

struct SphereData
{
    float3 center;
    uint radiusHalfAndColor; // bits 0..15 = FP16 radius, bits 16..31 = RGBA4444 color
};

struct CullParams
{
    uint activeSphereCount;
    uint enableCulling;
    uint enableSubPixelFallback;
    float viewportHeight;
};

struct PointVertexOutput
{
    float4 position : SV_Position;
    float pointSize : PSIZE;
    float4 color : COLOR0;
    float coverage : TEXCOORD0;
};

// Set 0, Binding 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

// Set 0, Binding 1: Point Spheres
[[vk::binding(1, 0)]] StructuredBuffer<SphereData> gPointSpheres;

// Set 0, Binding 2: Cull Params
[[vk::binding(2, 0)]] ConstantBuffer<CullParams> cullParams;

[shader("vertex")]
PointVertexOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    PointVertexOutput output;

    uint sphereID = instanceID;
    SphereData sphere = gPointSpheres[sphereID];

    float radius = unpackHalf(sphere.radiusHalfAndColor & 0xFFFFu);
    float r = abs(radius);

    // Center in view space
    float4 centerView = mul(scene.viewMatrix, float4(sphere.center, 1.0));

    // Clip space position
    output.position = mul(scene.projectionMatrix, centerView);
    output.pointSize = 1.0;

    // Calculate sub-pixel coverage based on projected screen area
    float zDepth = abs(centerView.z);
    float pixelDiameter = (r * scene.projectionMatrix[1][1] * cullParams.viewportHeight) / max(zDepth, 0.0001);

    // Circle area = pi/4 * d^2
    float area = 0.78539816339 * pixelDiameter * pixelDiameter;
    output.coverage = clamp(area, 0.0, 1.0);

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
