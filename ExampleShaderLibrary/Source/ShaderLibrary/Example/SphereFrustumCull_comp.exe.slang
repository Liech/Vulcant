import Render.Scene;

struct SphereData
{
    float3 center;
    float radius;
    uint colorPacked; // RGBA8 packed
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
    uint enableZSorting;
    uint pad0;
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

groupshared SphereData s_spheres[256];
groupshared float      s_depths[256];
groupshared uint       s_baseIdx;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint sphereID = dispatchThreadID.x;
    bool isThreadValid = (sphereID < cullParams.activeSphereCount);

    SphereData sphere;
    float depth = 1e9f;
    bool visible = false;

    if (isThreadValid)
    {
        sphere = gSpheres[sphereID];
        float r = abs(sphere.radius);

        visible = true;
        if (cullParams.enableCulling != 0)
        {
            float4x4 vp = mul(scene.projectionMatrix, scene.viewMatrix);
            float4 planes[6];
            planes[0] = vp[3] + vp[0]; // Left
            planes[1] = vp[3] - vp[0]; // Right
            planes[2] = vp[3] + vp[1]; // Bottom
            planes[3] = vp[3] - vp[1]; // Top
            planes[4] = vp[2];         // Near (Vulkan [0, 1] depth range)
            planes[5] = vp[3] - vp[2]; // Far

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
            float4 centerView = mul(scene.viewMatrix, float4(sphere.center, 1.0));
            depth = -centerView.z; // Distance from camera
        }
    }

    if (cullParams.enableZSorting == 0)
    {
        if (visible)
        {
            uint appendIndex;
            InterlockedAdd(gIndirectCommand[0].instanceCount, 1, appendIndex);
            gCulledSpheres[appendIndex] = sphere;
        }
        return;
    }

    // Threadgroup Bitonic Sort in Shared Memory
    s_spheres[groupIndex] = sphere;
    s_depths[groupIndex]  = visible ? depth : 1e9f;

    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for (uint k = 2; k <= 256; k <<= 1)
    {
        [unroll]
        for (uint j = k >> 1; j > 0; j >>= 1)
        {
            uint ixj = groupIndex ^ j;
            if (ixj > groupIndex)
            {
                bool ascending = ((groupIndex & k) == 0);
                float d1 = s_depths[groupIndex];
                float d2 = s_depths[ixj];

                if ((ascending && d1 > d2) || (!ascending && d1 < d2))
                {
                    s_depths[groupIndex] = d2;
                    s_depths[ixj]        = d1;

                    SphereData temp       = s_spheres[groupIndex];
                    s_spheres[groupIndex] = s_spheres[ixj];
                    s_spheres[ixj]        = temp;
                }
            }
            GroupMemoryBarrierWithGroupSync();
        }
    }

    if (groupIndex == 0)
    {
        uint count = 0;
        for (uint i = 0; i < 256; ++i)
        {
            if (s_depths[i] < 1e8f) count++;
            else break;
        }
        uint baseIdx = 0;
        if (count > 0)
        {
            InterlockedAdd(gIndirectCommand[0].instanceCount, count, baseIdx);
        }
        s_baseIdx = baseIdx;
    }

    GroupMemoryBarrierWithGroupSync();

    float sortedDepth = s_depths[groupIndex];
    if (sortedDepth < 1e8f)
    {
        gCulledSpheres[s_baseIdx + groupIndex] = s_spheres[groupIndex];
    }
}
