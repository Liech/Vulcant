import Render.Scene;

struct SphereData
{
    float3 center;
    float radius;
    uint colorPacked; // RGBA8 packed
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

[shader("vertex")]
SphereVertexOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    SphereVertexOutput output;

    uint sphereID = instanceID;
    uint cornerID = vertexID % 6;
    float2 quadPos = quadOffsets[cornerID];

    SphereData sphere = gSpheres[sphereID];
    float r = abs(sphere.radius);

    // Center in view space
    float4 centerView = mul(scene.viewMatrix, float4(sphere.center, 1.0));

    // Billboard quad expansion in view space (camera-facing)
    float3 cornerView = centerView.xyz + float3(quadPos * r, 0.0);

    output.position = mul(scene.projectionMatrix, float4(cornerView, 1.0));
    output.uv = quadPos;
    output.sphereCenterView = centerView.xyz;
    output.sphereRadius = sphere.radius;

    // Pass projection parameters for exact depth calculation in fragment shader without needing descriptor sets
    output.projParams = float4(scene.projectionMatrix[2][2], scene.projectionMatrix[2][3], scene.projectionMatrix[3][2], scene.projectionMatrix[3][3]);

    // Unpack RGBA8 color
    output.color = float4(
        float(sphere.colorPacked & 0xFF) / 255.0,
        float((sphere.colorPacked >> 8) & 0xFF) / 255.0,
        float((sphere.colorPacked >> 16) & 0xFF) / 255.0,
        float((sphere.colorPacked >> 24) & 0xFF) / 255.0
    );

    return output;
}
