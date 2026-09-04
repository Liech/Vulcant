module Light;

namespace Render {
public struct Light {
    public float3 position;
    public float type;
    public float3 direction;
    public float range;
    public float3 color;
    public float energy;
    public float cos_inner;
    public float cos_outer;
    public float attenuation;
    public float padding;
};
}