// Slang uses a more modern, HLSL-like syntax with structured buffers
// and specific attribute descriptors for compute shaders.

import Render.Scene;
import Render.Light;
using namespace Render;

// --- Bindings ---

// Set 0: G-Buffer
[[vk::binding(0, 0)]] Sampler2D colorTexture;
[[vk::binding(1, 0)]] Sampler2D depthTexture;
[[vk::binding(2, 0)]] Sampler2D normalTexture;

// Set 1: Output
[format("rgba32f")]
[[vk::binding(0, 1)]] RWTexture2D<float4> resultImage;

// Set 2: Scene Data
[[vk::binding(0, 2)]] ConstantBuffer<Scene> scene;

// Set 3: Light Data
[[vk::binding(0, 3)]] StructuredBuffer<Light> lights;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 texelCoord = dispatchThreadID.xy;
    
    uint2 size;
    colorTexture.GetDimensions(size.x, size.y);

    if (texelCoord.x >= size.x || texelCoord.y >= size.y) 
        return;

    // Slang/HLSL style sampling (texelFetch equivalent)
    float depth = depthTexture.Load(int3(texelCoord, 0)).r;
    
    if (depth >= 1.0) {
        resultImage[texelCoord] = float4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    float4 albedoSample = colorTexture.Load(int3(texelCoord, 0));
    float3 albedo = albedoSample.rgb;
    
    // 1. Unpack Normal (View Space)
    float3 viewNormal = normalize(normalTexture.Load(int3(texelCoord, 0)).xyz * 2.0 - 1.0);

    // 2. Reconstruct View Space Position
    float2 uv = (float2(texelCoord) + 0.5) / float2(size);
    float4 ndcPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    // In Slang/HLSL, matrix multiplication order is typically vector * matrix or mul()
    float4 viewPosFull = mul(scene.invProjectionMatrix, ndcPos);
    float3 viewPos = viewPosFull.xyz / viewPosFull.w;

    // 3. View Direction in View Space
    float3 viewDir = normalize(-viewPos);

    float3 totalLighting = albedo * 0.03; // Ambient

    // 4. Light Loop
    uint lightCount;
    uint dummy;
    lights.GetDimensions(lightCount, dummy);

    for (uint i = 0; i < lightCount; ++i) {
        Light light = lights[i];
        
        float3 lightDirView;
        float attenuation = 1.0;

        if (light.type == 0.0) { 
            // Directional: World to View transformation
            // Cast to float3x3 for the normal matrix
            lightDirView = normalize(mul((float3x3)scene.viewMatrix, -light.direction));
        } else { 
            // Omni: World Position to View Position
            float4 lightPosView = mul(scene.viewMatrix, float4(light.position, 1.0));
            float3 lightVec = lightPosView.xyz - viewPos;
            
            float dist = length(lightVec);
            lightDirView = normalize(lightVec);
            
            attenuation = clamp(1.0 - (dist / light.range), 0.0, 1.0);
            attenuation *= attenuation;
        }

        // --- Diffuse (Lambert) ---
        float diff = max(dot(viewNormal, lightDirView), 0.0);
        float3 diffuse = diff * albedo * light.color * light.energy;

        // --- Specular (Blinn-Phong) ---
        float3 halfwayDir = normalize(lightDirView + viewDir);
        float spec = pow(max(dot(viewNormal, halfwayDir), 0.0), 32.0);
        float3 specular = spec * light.color * light.energy;

        totalLighting += (diffuse + specular) * attenuation;
    }

    // Basic Reinhard Tone Mapping
    float3 finalColor = totalLighting / (totalLighting + float3(1.0));
    resultImage[texelCoord] = float4(finalColor, albedoSample.a);
}