#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>
#include <cstdint>
#include <string>
#include <vector>

struct SphereData
{
    glm::vec3 center;             // 12 bytes
    uint32_t  radiusHalfAndColor; // 4 bytes: bits 0..15 = FP16 radius, bits 16..31 = RGBA4444 color

    static uint16_t packFloatToHalf(float val)
    {
        return static_cast<uint16_t>(glm::packHalf2x16(glm::vec2(val, 0.0f)) & 0xFFFF);
    }

    static uint16_t packColorRGBA4444(uint32_t rgba8888)
    {
        uint32_t r = (rgba8888 & 0xFF) >> 4;
        uint32_t g = ((rgba8888 >> 8) & 0xFF) >> 4;
        uint32_t b = ((rgba8888 >> 16) & 0xFF) >> 4;
        uint32_t a = ((rgba8888 >> 24) & 0xFF) >> 4;
        return static_cast<uint16_t>(r | (g << 4) | (b << 8) | (a << 12));
    }

    void setRadiusAndColor(float radius, uint32_t rgba8888)
    {
        uint32_t rHalf = packFloatToHalf(radius);
        uint32_t c16   = packColorRGBA4444(rgba8888);
        radiusHalfAndColor = rHalf | (c16 << 16);
    }

    float getRadius() const
    {
        uint16_t rHalf = static_cast<uint16_t>(radiusHalfAndColor & 0xFFFF);
        return glm::unpackHalf2x16(static_cast<uint32_t>(rHalf)).x;
    }

    uint32_t getColorRGBA8888() const
    {
        uint32_t c16 = radiusHalfAndColor >> 16;
        uint32_t r   = (c16 & 0x0F) * 17;
        uint32_t g   = ((c16 >> 4) & 0x0F) * 17;
        uint32_t b   = ((c16 >> 8) & 0x0F) * 17;
        uint32_t a   = ((c16 >> 12) & 0x0F) * 17;
        return r | (g << 8) | (b << 16) | (a << 24);
    }
};

static_assert(sizeof(SphereData) == 16, "SphereData layout must be 16 bytes");

struct SphereInitData
{
    alignas(16) glm::vec3 basePosition;
    float                orbitSpeed;
    float                orbitRadius;
    float                baseTheta;
    float                radiusScale;
    uint32_t             packedColor;
};

namespace Vulcant
{
    namespace Examples
    {
        class SphereScene
        {
          public:
            virtual ~SphereScene() = default;

            virtual std::string getName() const = 0;
            virtual void        generate(size_t count, float baseRadius, std::vector<SphereData>& outSpheres, std::vector<SphereInitData>& outInitData) = 0;
        };
    }
}
