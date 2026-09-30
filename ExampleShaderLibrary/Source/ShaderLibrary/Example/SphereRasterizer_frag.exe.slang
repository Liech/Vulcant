struct FragmentInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 sphereCenterView : TEXCOORD1;
    float sphereRadius : PSIZE;
    float4 color : COLOR0;
    float4 projParams : TEXCOORD2;
};

struct FragmentOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float depth : SV_Depth;
};

[shader("fragment")]
FragmentOutput main(FragmentInput input)
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

    if (input.sphereRadius < 0.0)
    {
        // --- 3D Gaussian Splatting Mode ---
        // Smooth radial Gaussian radiance decay from center to rim
        float gaussian = exp(-3.0 * distSq);
        outColor.rgb *= gaussian;
        outColor.a   *= gaussian;

        // Soft, camera-facing normal for smooth volumetric splat shading
        normalView = float3(0.0, 0.0, 1.0);

        // Splat plane at sphere center depth (avoids hard shell intersecting edges)
        hitPosView = input.sphereCenterView;
    }
    else
    {
        // --- Opaque Sphere Impostor Mode ---
        // Hemispherical 3D normal for analytical curved sphere shading
        float z = sqrt(max(0.0, 1.0 - distSq));
        normalView = normalize(float3(input.uv.x, input.uv.y, z));

        // Surface point on front hemisphere for pixel-perfect spherical depth
        hitPosView = input.sphereCenterView + normalView * r;
    }

    // Transform view space hit position to clip space depth using passed projection parameters
    float clipZ = input.projParams.x * hitPosView.z + input.projParams.y;
    float clipW = input.projParams.z * hitPosView.z + input.projParams.w;
    float clipDepth = (abs(clipW) > 1e-6) ? (clipZ / clipW) : (input.position.z);

    FragmentOutput output;
    output.color = outColor;
    output.normal = float4(normalView * 0.5 + 0.5, 1.0);
    output.depth = clipDepth;

    return output;
}
