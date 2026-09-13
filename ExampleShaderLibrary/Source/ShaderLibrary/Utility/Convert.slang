module Convert;

interface Convert<A,B>
{
    B convert(A value);
};

struct convert_float2float : Convert<float, float>
{
    float convert(float value) { return value; }
};

struct convert_uint2float : Convert<uint, float>
{
    float convert(uint value) { return float(value & 0xFF) / 255.0f; }
};

struct convert_double2float : Convert<double, float>
{
    float convert(double value) { return (float)value; }
};

struct convert_bool2float : Convert<bool, float>
{
    float convert(bool value) { return value ? 1.0f : 0.0f; }
};

struct convert_float32float_length : Convert<float3, float>
{
    float convert(float3 value) { return length(value); }
};

struct convert_float32float_lengthsquared : Convert<float3, float>
{
    float convert(float3 value) { return dot(value,value); }
};