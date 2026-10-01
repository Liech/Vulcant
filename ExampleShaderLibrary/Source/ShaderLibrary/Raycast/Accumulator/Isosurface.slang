module Isosurface;

import Raycast.VolumeAccumulator;

namespace Raycast
{
public struct Isosurface : Raycast.VolumeAccumulator<float>
{
    public float isosurface = 0.1f;

    private float previousDistance = 0.0f;
    private float previousValue = 0.0f;

    public float currentValue = 0.0f;
    public AccumulatorError errorCode = AccumulatorError.None;

    public [mutating] void initialize()
    {
        currentValue = 0.0f;
        previousDistance = 0.0f;
        previousValue = 0.0f;
        errorCode = AccumulatorError.None;
    }

    public [mutating] bool accumulate(float value, float start, float end)
    {
        if (previousDistance == 0.0)
        {
            previousDistance = start;
            previousValue = value;
            currentValue = 0.01f;
            errorCode = Raycast.AccumulatorError.StackOverflow;
        }
        else
        {
            bool passUp = previousValue < isosurface && value > isosurface;
            bool passDown = previousValue > isosurface && value < isosurface;
            if (passUp || passDown)
            {
                currentValue = 1.0f;
                errorCode = AccumulatorError.None;
                return true;
            }
            previousValue = value;
            previousDistance = start;
            currentValue = 0.05f;
            errorCode = Raycast.AccumulatorError.MaxDepthReached;
        }
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