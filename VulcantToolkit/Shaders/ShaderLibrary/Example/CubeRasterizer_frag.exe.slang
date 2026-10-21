import Render.Scene;
import Example.BillboardRasterizer;

// Set 0: Scene Data
[[vk::binding(0, 0)]] ConstantBuffer<Render.Scene> scene;

public struct CubeVertexOutput
{
    public float4 position : SV_Position;
    public float4 color : COLOR0;
    public float3 rayOriginObj : TEXCOORD0;
    public float3 rayDirObj : TEXCOORD1;
    public float edgeLength : PSIZE;
    public float4 projParams : TEXCOORD2;
};

[shader("fragment")]
RasterizerOutput main(CubeVertexOutput input)
{
    float halfL = input.edgeLength * 0.5;
    float3 boxMin = float3(-halfL);
    float3 boxMax = float3(halfL);

    float3 rayOrigin = input.rayOriginObj;
    float3 rayDir = normalize(input.rayDirObj);

    // Slab intersection
    float3 invDir = 1.0 / rayDir;
    float3 t0 = (boxMin - rayOrigin) * invDir;
    float3 t1 = (boxMax - rayOrigin) * invDir;

    float3 tMin = min(t0, t1);
    float3 tMax = max(t0, t1);

    float tNear = max(max(tMin.x, tMin.y), tMin.z);
    float tFar  = min(min(tMax.x, tMax.y), tMax.z);

    if (tNear > tFar || tFar < 0.0)
    {
        discard;
    }

    float tHit = (tNear >= 0.0) ? tNear : tFar;

    // Normal in world space
    float3 normalWorld = float3(0.0);
    if (tNear >= 0.0)
    {
        if (tNear == tMin.x) normalWorld = float3(-sign(rayDir.x), 0.0, 0.0);
        else if (tNear == tMin.y) normalWorld = float3(0.0, -sign(rayDir.y), 0.0);
        else normalWorld = float3(0.0, 0.0, -sign(rayDir.z));
    }
    else
    {
        if (tFar == tMax.x) normalWorld = float3(-sign(rayDir.x), 0.0, 0.0);
        else if (tFar == tMax.y) normalWorld = float3(0.0, -sign(rayDir.y), 0.0);
        else normalWorld = float3(0.0, 0.0, -sign(rayDir.z));
    }

    // Normal in view space
    float3 normalView = normalize(mul((float3x3)scene.viewMatrix, normalWorld));

    // Hit position in world space
    float3 hitWorld = scene.cameraPos.xyz + rayDir * tHit;
    float4 hitView = mul(scene.viewMatrix, float4(hitWorld, 1.0));

    // Clip depth
    float clipZ = input.projParams.x * hitView.z + input.projParams.y;
    float clipW = input.projParams.z * hitView.z + input.projParams.w;
    float clipDepth = (abs(clipW) > 1e-6) ? (clipZ / clipW) : input.position.z;

    RasterizerOutput output;
    output.color = input.color;
    output.normal = float4(normalView * 0.5 + 0.5, 1.0);
    output.depth = clipDepth;

    return output;
}
