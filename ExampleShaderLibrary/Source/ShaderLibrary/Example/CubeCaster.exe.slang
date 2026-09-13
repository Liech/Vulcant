// --- Types ---

import Raycast.Ray;
import Raycast.Intersect;
import Render.Scene;

// --- Bindings ---

// Set 0: G-Buffer Outputs
[format("rgba32f")]
[[vk::binding(0, 0)]] RWTexture2D<float4> resultImage;

[format("r32f")]
[[vk::binding(1, 0)]] RWTexture2D<float> depthImage;

[format("rgba16f")]
[[vk::binding(2, 0)]] RWTexture2D<float4> normalImage;

// Set 1: Scene Data
[[vk::binding(0, 1)]] ConstantBuffer<Render.Scene> scene;

// --- Main Entry Point ---

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 pixel_coords = dispatchThreadID.xy;
    
    uint2 dims;
    resultImage.GetDimensions(dims.x, dims.y);
    
    if(pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) return;

    float2 uv = (float2(pixel_coords) + 0.5) / float2(dims);
    Raycast.Ray r = scene.get_ray(uv);

    float3 boxMin = float3(-0.5);
    float3 boxMax = float3(0.5);
    
    float4 finalColor = float4(0.0); 
    float finalDepth = 1.0; 
    float3 out_normal = float3(0.0);

    float2 t_hit = Raycast.hit_aabb(boxMin, boxMax, r);
    float t = t_hit.x;

    if (t_hit.x <= t_hit.y && t_hit.y > 0.0) {
        if (t < 0.0) t = 0.0; 

        float3 hit_world = r.origin + r.direction * t;
        
        // Transform hit point to clip space for depth calculation
        float4 hit_view = mul(scene.viewMatrix, float4(hit_world, 1.0));
        float4 hit_clip = mul(scene.projectionMatrix, hit_view);
        
        finalDepth = hit_clip.z / hit_clip.w;
        finalColor = float4(1.0, 0.0, 0.0, 1.0);

        float3 world_normal = Raycast.get_aabb_normal(hit_world, boxMin, boxMax);
        // Cast mat4 to float3x3 for normal transformation
        out_normal = mul((float3x3)scene.viewMatrix, world_normal);
    }

    float3 packed_normal = out_normal * 0.5 + 0.5;
    
    normalImage[pixel_coords] = float4(packed_normal, 1.0);
    resultImage[pixel_coords] = finalColor;
    depthImage[pixel_coords] = finalDepth;
}