module Scene;

import Raycast.Ray;

namespace Render {
public struct Scene {
    public float4x4 viewMatrix;
    public float4x4 projectionMatrix;
    public float4x4 invProjectionMatrix;
    public float4x4 invViewMatrix;
    public float4 cameraPos;
    public float4 lightPos;

    public Raycast.Ray get_ray(float2 uv) {
        float2 ndc_xy = uv * 2.0 - 1.0;
        float4 target_ndc = float4(ndc_xy, 0.0, 1.0);
        float4 target_world = mul(this.invViewMatrix, mul(this.invProjectionMatrix, target_ndc));
        target_world /= target_world.w;
        Raycast.Ray r;
        r.origin = this.cameraPos.xyz;
        r.direction = normalize(target_world.xyz - r.origin);
        return r;
    }
};


}