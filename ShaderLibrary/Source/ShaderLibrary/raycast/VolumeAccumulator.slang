module VolumeAccumulator;

namespace Raycast
{
public enum AccumulatorError : uint
{
    None = 0,
    StackOverflow = 1, 
    MaxDepthReached = 2,
}

public interface VolumeAccumulator<VoxelData>
{
    public [mutating] void initialize();
    public [mutating] bool accumulate(VoxelData voxel, float start, float end); // true means stop
    public [mutating] void finalize();
    public [mutating] void error(AccumulatorError);
}
}