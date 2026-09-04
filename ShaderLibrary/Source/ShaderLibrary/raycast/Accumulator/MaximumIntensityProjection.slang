module MaximumIntensityProjection;

import VolumeAccumulator;

namespace Raycast
{
public struct MaximumIntensityProjection : Raycast.VolumeAccumulator<float>
{
    public float currentValue = -10000.0f;
    public AccumulatorError errorCode = AccumulatorError.None;

    public [mutating] void initialize()
    {
        currentValue = -10000.0;
        errorCode = AccumulatorError.None;
    }

    public [mutating] bool accumulate(float value, float start, float end)
    {
        currentValue = max(currentValue, value);
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