module AccumulateIntensity;

import Raycast.VolumeAccumulator;

namespace Raycast
{
public struct AccumulateIntensity : Raycast.VolumeAccumulator<float>
{
    public float currentValue = 0.0f;
    public AccumulatorError errorCode = AccumulatorError.None;

    public [mutating] void initialize()
    {
        currentValue = 0.0f;
        errorCode = AccumulatorError.None;
    }

    public [mutating] bool accumulate(float value, float start, float end)
    {
        currentValue += (end - start) * value;
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