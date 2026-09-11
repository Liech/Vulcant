#pragma once

namespace Vulcant::Rendering
{
    struct alignas(16) Light
    {
        // Transform & Direction
        float position[3]; // Used by Omni and Spot
        float type;        // 0: Directional, 1: Omni, 2: Spot

        float direction[3]; // Used by Directional and Spot
        float range;        // Max distance (Omni/Spot)

        // Color & Intensity
        float color[3]; // RGB
        float energy;   // Multiplier

        // Spot Light Specifics
        float cos_inner_angle; // Pre-calculated cos(inner)
        float cos_outer_angle; // Pre-calculated cos(outer)
        float attenuation;     // Constant/Linear/Quadratic factor
        float padding;         // Keep it 16-byte aligned
    };
    static_assert(sizeof(Light) == 64, "Light struct size must be 64 bytes for std430 compatibility.");
}