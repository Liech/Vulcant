import Example.BillboardRasterizer;

struct FragmentInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 sphereCenterView : TEXCOORD1;
    float sphereRadius : PSIZE;
    float4 color : COLOR0;
    float4 projParams : TEXCOORD2;
};

[shader("fragment")]
RasterizerOutput main(FragmentInput input)
{
    float distSq = dot(input.uv, input.uv);
    if (distSq > 1.0)
    {
        discard;
    }

    float r = abs(input.sphereRadius);
    float4 outColor = input.color;
    float3 normalView;
    float3 hitPosView;

    // Hemispherical 3D normal for analytical curved sphere shading
    float z = sqrt(max(0.0, 1.0 - distSq));
    normalView = normalize(float3(input.uv.x, input.uv.y, z));

    // Surface point on front hemisphere for pixel-perfect spherical depth
    hitPosView = input.sphereCenterView + normalView * r;

    // Transform view space hit position to clip space depth using passed projection parameters
    float clipZ = input.projParams.x * hitPosView.z + input.projParams.y;
    float clipW = input.projParams.z * hitPosView.z + input.projParams.w;
    float clipDepth = (abs(clipW) > 1e-6) ? (clipZ / clipW) : (input.position.z);

    RasterizerOutput output;
    output.color = outColor;
    output.normal = float4(normalView * 0.5 + 0.5, 1.0);
    output.depth = clipDepth;

    return output;
}
