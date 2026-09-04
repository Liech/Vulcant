// 1. Image declaration with explicit format to fix "WriteWithoutFormat" error
[[vk::image_format("rgba32f")]]
[[vk::binding(0, 0)]] 
RWTexture2D<float4> resultImage;

import raycast.Ray;
import raycast.Intersect;
using namespace Raycast;

// 2. Buffer declaration: GLSL 'buffer' maps to 'StructuredBuffer' or 'RWStructuredBuffer'
// This fixes the VK_DESCRIPTOR_TYPE_STORAGE_BUFFER mismatch
struct Scene {
    float4x4 viewMatrix;
    float4 lightPos;
};

[[vk::binding(0, 1)]]
StructuredBuffer<Scene> scene; 

[shader("compute")]
[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID) {
    uint width, height;
    resultImage.GetDimensions(width, height);
    uint2 dims = uint2(width, height);

    uint2 pixel_coords = threadID.xy;
    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) return;

    float2 uv = (float2(pixel_coords) / float2(dims)) * 2.0f - 1.0f;
    //uv.y = -uv.y; // Standard HLSL/Slang Y-flip; remove if image is upside down
    uv.x *= (float)dims.x / (float)dims.y;

    float3 rayOrigin = float3(0.0, 0.0, 0.0);
    float3 rayDir = normalize(float3(uv, -1.5));

    // Accessing 'scene' as a StructuredBuffer requires index [0] 
    // to match the GLSL 'layout(...) buffer scene' behavior
    float3 worldOrigin = mul(scene[0].viewMatrix, float4(rayOrigin, 1.0)).xyz;
    float3 worldDir = normalize(mul(scene[0].viewMatrix, float4(rayDir, 0.0)).xyz);
    
    Ray ray = { worldOrigin, worldDir };

    float3 sphereCenter = float3(0.0, 0.0, 0.0);
    float radius = 0.5;
    float4 finalColor = float4(0.0); 

    float t = hit_sphere(sphereCenter, radius, ray);
    if (t > 0.0) {
        float3 hitPoint = ray.origin + t * ray.direction;
        float3 normal = normalize(hitPoint - sphereCenter);
        float3 lightDir = normalize(scene[0].lightPos.xyz - hitPoint);
        float diff = max(dot(normal, lightDir), 0.0);
        float3 sphereColor = float3(0.8, 0.2, 0.2);
        finalColor = float4((sphereColor * 0.1) + (sphereColor * diff), 1.0);
    }

    resultImage[pixel_coords] = finalColor;
}