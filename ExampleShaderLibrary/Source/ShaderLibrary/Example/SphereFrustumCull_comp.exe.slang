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

groupshared uint       s_binCounts[16];
groupshared uint       s_binOffsets[16];
groupshared SphereData s_outSpheres[256];
groupshared uint       s_globalBaseIdx;
groupshared uint       s_totalGroupVisible;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint sphereID = dispatchThreadID.x;
    bool isThreadValid = (sphereID < cullParams.activeSphereCount);

    SphereData sphere = {};
    float viewDepth = 1e9f;
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
            planes[4] = vp[2];         // Near
            planes[5] = vp[3] - vp[2]; // Far

            for (int i = 0; i < 6; ++i)
            {
                float4 p = planes[i];
                float len = length(p.xyz);
                if (len > 1e-6) p /= len;

                if (dot(p.xyz, sphere.center) + p.w < -r)
                {
                    visible = false;
                    break;
                }
            }
        }

        if (visible)
        {
            float4 centerView = mul(scene.viewMatrix, float4(sphere.center, 1.0));
            viewDepth = -centerView.z;
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

    // Fast 16-Bucket Front-to-Back Depth Binning
    uint bin = 15u;
    if (visible && viewDepth < 1e8f)
    {
        float normD = saturate((viewDepth - 0.1f) / 150.0f);
        bin = clamp(uint(normD * 16.0f), 0u, 15u);
    }

    if (groupIndex < 16)
    {
        s_binCounts[groupIndex] = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    if (visible)
    {
        InterlockedAdd(s_binCounts[bin], 1u);
    }

    GroupMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        uint sum = 0;
        for (uint b = 0; b < 16; ++b)
        {
            s_binOffsets[b] = sum;
            sum += s_binCounts[b];
            s_binCounts[b] = 0; // reset to 0 for atomic local scatter
        }

        s_totalGroupVisible = sum;

        uint baseIdx = 0;
        if (sum > 0)
        {
            InterlockedAdd(gIndirectCommand[0].instanceCount, sum, baseIdx);
        }
        s_globalBaseIdx = baseIdx;
    }

    GroupMemoryBarrierWithGroupSync();

    if (visible)
    {
        uint localOffset;
        InterlockedAdd(s_binCounts[bin], 1u, localOffset);
        uint destSlot = s_binOffsets[bin] + localOffset;
        s_outSpheres[destSlot] = sphere;
    }

    GroupMemoryBarrierWithGroupSync();

    // Parallel write from shared memory to global culled buffer
    if (groupIndex < s_totalGroupVisible)
    {
        gCulledSpheres[s_globalBaseIdx + groupIndex] = s_outSpheres[groupIndex];
    }
}
