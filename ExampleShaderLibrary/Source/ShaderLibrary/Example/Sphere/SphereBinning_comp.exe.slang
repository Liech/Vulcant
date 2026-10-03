import Render.Scene;
import Utility.HalfPacking;

struct SphereData
{
    float3 center;
    uint radiusHalfAndColor; // FP16 radius (lower 16 bits) + RGBA4444 color (upper 16 bits)
};

struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

// Set 0, Binding 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

// Set 0, Binding 1: Compact Culled Spheres from Pass 1
[[vk::binding(1, 0)]] StructuredBuffer<SphereData> gCulledSpheres;

// Set 0, Binding 2: Indirect Command (reads instanceCount)
[[vk::binding(2, 0)]] StructuredBuffer<DrawIndirectCommand> gIndirectCommand;

// Set 0, Binding 3: Global Bin Counts from Pass 1
[[vk::binding(3, 0)]] StructuredBuffer<uint> gBinCounts;

// Set 0, Binding 4: Output Contiguous Sorted Culled Spheres
[[vk::binding(4, 0)]] RWStructuredBuffer<SphereData> gSortedCulledSpheres;

groupshared uint s_binOffsets[16];
groupshared uint s_binWriteCounts[16];

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint groupIndex : SV_GroupIndex)
{
    // Thread 0..15 initializes bin write counts
    if (groupIndex < 16)
    {
        s_binWriteCounts[groupIndex] = 0;
    }

    // Thread 0 computes prefix sums for the 16 bins
    if (groupIndex == 0)
    {
        uint sum = 0;
        for (uint b = 0; b < 16; ++b)
        {
            s_binOffsets[b] = sum;
            sum += gBinCounts[b];
        }
    }

    GroupMemoryBarrierWithGroupSync();

    uint totalCulled = gIndirectCommand[0].instanceCount;

    // Strided loop over all culled spheres using the 256 threads of this single workgroup
    for (uint i = groupIndex; i < totalCulled; i += 256)
    {
        SphereData sphere = gCulledSpheres[i];

        // Compute view depth
        float4 centerView = mul(scene.viewMatrix, float4(sphere.center, 1.0));
        float viewDepth = -centerView.z;

        // Map view depth to 16 global buckets [0..15]
        float normDepth = saturate((viewDepth - 0.1f) / 150.0f);
        uint bin = clamp(uint(normDepth * 16.0f), 0u, 15u);

        uint localBinIndex;
        InterlockedAdd(s_binWriteCounts[bin], 1u, localBinIndex);

        uint destSlot = s_binOffsets[bin] + localBinIndex;
        gSortedCulledSpheres[destSlot] = sphere;
    }
}
