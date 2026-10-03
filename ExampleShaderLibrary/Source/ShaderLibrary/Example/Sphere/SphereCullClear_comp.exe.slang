struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

// Binding 0: Indirect Draw Command
[[vk::binding(0, 0)]] RWStructuredBuffer<DrawIndirectCommand> gIndirectCommand;

// Binding 1: Global Bin Counts
[[vk::binding(1, 0)]] RWStructuredBuffer<uint> gBinCounts;

[shader("compute")]
[numthreads(16, 1, 1)]
void main(uint groupIndex : SV_GroupIndex)
{
    if (groupIndex == 0)
    {
        gIndirectCommand[0].vertexCount = 6;
        gIndirectCommand[0].instanceCount = 0;
        gIndirectCommand[0].firstVertex = 0;
        gIndirectCommand[0].firstInstance = 0;
    }

    if (groupIndex < 16)
    {
        gBinCounts[groupIndex] = 0;
    }
}
