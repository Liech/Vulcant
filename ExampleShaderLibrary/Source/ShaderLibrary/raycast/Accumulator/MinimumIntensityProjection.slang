module MinimumIntensityProjection;

import Raycast.VolumeAccumulator;

namespace Raycast
{
public struct MinimumIntensityProjection : Raycast.VolumeAccumulator<float>
{
    public float currentValue = 0.0f;
    public AccumulatorError errorCode = AccumulatorError.None;

    public [mutating] void initialize()
    {
        currentValue = 1000000.0;
        errorCode = AccumulatorError.None;
    }

    public [mutating] bool accumulate(float value, float start, float end)
    {
        currentValue = min(currentValue, value);
        return false;
    }

    public [mutating] void finalize() 
    {
    }

    public [mutating] void error(AccumulatorError err)
    {
        errorCode = err;
    }
};
}