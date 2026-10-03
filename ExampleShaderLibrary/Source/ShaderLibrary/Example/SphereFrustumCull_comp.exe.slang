import Render.Scene;
import Utility.HalfPacking;

struct SphereData
{
    float3 center;
    uint radiusHalfAndColor; // bits 0..15 = FP16 radius, bits 16..31 = RGBA4444 color
};

struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct CullParams
{
    uint activeSphereCount;
    uint enableCulling;
    uint pad0;
    uint pad1;
};

// Set 0, Binding 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

// Set 0, Binding 1: Input Spheres
[[vk::binding(1, 0)]] StructuredBuffer<SphereData> gSpheres;

// Set 0, Binding 2: Cull Parameters
[[vk::binding(2, 0)]] ConstantBuffer<CullParams> cullParams;

// Set 0, Binding 3: Output Culled Spheres
[[vk::binding(3, 0)]] RWStructuredBuffer<SphereData> gCulledSpheres;

// Set 0, Binding 4: Output Indirect Draw Command
[[vk::binding(4, 0)]] RWStructuredBuffer<DrawIndirectCommand> gIndirectCommand;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint sphereID = dispatchThreadID.x;
    if (sphereID >= cullParams.activeSphereCount)
        return;

    SphereData sphere = gSpheres[sphereID];
    float radius = unpackHalf(sphere.radiusHalfAndColor & 0xFFFFu);
    float r = abs(radius);

    // View-projection matrix
    float4x4 vp = mul(scene.projectionMatrix, scene.viewMatrix);

    // Extract 6 view-frustum planes from VP matrix
    float4 planes[6];
    planes[0] = vp[3] + vp[0]; // Left
    planes[1] = vp[3] - vp[0]; // Right
    planes[2] = vp[3] + vp[1]; // Bottom
    planes[3] = vp[3] - vp[1]; // Top
    planes[4] = vp[2];         // Near (Vulkan [0, 1] depth range)
    planes[5] = vp[3] - vp[2]; // Far

    bool visible = true;
    if (cullParams.enableCulling != 0u)
    {
        for (int i = 0; i < 6; ++i)
        {
            float4 p = planes[i];
            float len = length(p.xyz);
            p /= len;

            float dist = dot(p.xyz, sphere.center) + p.w;
            if (dist < -r)
            {
                visible = false;
                break;
            }
        }
    }

    if (visible)
    {
        uint appendIndex;
        InterlockedAdd(gIndirectCommand[0].instanceCount, 1u, appendIndex);
        gCulledSpheres[appendIndex] = sphere;
    }
}
