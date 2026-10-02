struct SphereInitData
{
    float3 basePosition;
    float orbitSpeed;
    float orbitRadius;
    float baseTheta;
    float radiusScale;
    uint packedColor;
};

struct SphereData
{
    float3 center;
    float radius;
    uint colorPacked;
};

struct SphereAnimUniforms
{
    float time;
    float animSpeed;
    float baseRadius;
    float radiusSign; // 1.0f or -1.0f
    uint activeSphereCount;
    uint pad0;
    uint pad1;
    uint pad2;
};

[[vk::binding(0, 0)]] ConstantBuffer<SphereAnimUniforms> uniforms;
[[vk::binding(1, 0)]] StructuredBuffer<SphereInitData> gInitSpheres;
[[vk::binding(2, 0)]] RWStructuredBuffer<SphereData> gOutputSpheres;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint sphereID = dispatchThreadID.x;
    if (sphereID >= uniforms.activeSphereCount)
        return;

    SphereInitData initData = gInitSpheres[sphereID];

    float speed = initData.orbitSpeed * uniforms.animSpeed;
    float baseTheta = initData.baseTheta;
    float dist = initData.orbitRadius;

    float currentTheta = baseTheta + speed * uniforms.time * 0.3f;
    float x = dist * cos(currentTheta);
    float z = dist * sin(currentTheta);
    float y = initData.basePosition.y + 0.15f * sin(uniforms.time * 2.0f + dist * 2.0f);

    float sphereRad = uniforms.radiusSign * uniforms.baseRadius * initData.radiusScale;

    SphereData outData;
    outData.center = float3(x, y, z);
    outData.radius = sphereRad;
    outData.colorPacked = initData.packedColor;

    gOutputSpheres[sphereID] = outData;
}
