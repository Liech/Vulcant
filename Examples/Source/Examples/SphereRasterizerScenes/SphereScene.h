#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct SphereData
{
    alignas(16) glm::vec3 center;
    float        radius;
    unsigned int colorPacked; // RGBA8 packed
};

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
