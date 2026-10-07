import Render.Scene;
import Example.BillboardRasterizer;

// Set 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

// Set 1: Spheres / Cubes
[[vk::binding(0, 1)]] StructuredBuffer<SphereData> gCubes;

public struct CubeVertexOutput
{
    public float4 position : SV_Position;
    public float4 color : COLOR0;
    public float3 rayOriginObj : TEXCOORD0;
    public float3 rayDirObj : TEXCOORD1;
    public float edgeLength : PSIZE;
    public float4 projParams : TEXCOORD2;
};

[shader("vertex")]
CubeVertexOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    CubeVertexOutput output;

    uint cubeID = instanceID;
    uint cornerID = vertexID % 6u;
    float2 quadPos = quadOffsets[cornerID];

    SphereData cube = gCubes[cubeID];
    float edgeLen = cube.getRadius();
    float L = abs(edgeLen);

    // Center in view space
    float4 centerView = mul(scene.viewMatrix, float4(cube.center, 1.0));

    // Bounding radius of cube with edge length L is L * sqrt(3)/2 ≈ L * 0.86603.
    // Add margin (0.87) to fully enclose the cube from all view angles.
    float r_bound = L * 0.87;

    // Billboard quad expansion in view space (camera-facing)
    float3 cornerView = centerView.xyz + float3(quadPos * r_bound, 0.0);

    output.position = mul(scene.projectionMatrix, float4(cornerView, 1.0));
    output.color = cube.getColor();
    output.edgeLength = L;

    // Pass projection parameters for exact depth calculation
    output.projParams = float4(scene.projectionMatrix[2][2], scene.projectionMatrix[2][3], scene.projectionMatrix[3][2], scene.projectionMatrix[3][3]);

    // Unproject billboard corner to world space
    float4 cornerWorld = mul(scene.invViewMatrix, float4(cornerView, 1.0));

    // Ray origin in object space (centered at cube.center)
    output.rayOriginObj = scene.cameraPos.xyz - cube.center;

    // Ray direction in object space
    output.rayDirObj = cornerWorld.xyz - scene.cameraPos.xyz;

    return output;
}
