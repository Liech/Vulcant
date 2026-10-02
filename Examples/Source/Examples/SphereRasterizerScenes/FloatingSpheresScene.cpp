#include "Examples/SphereRasterizerScenes/FloatingSpheresScene.h"
#include "Examples/SphereRasterizerScenes/ConcentricRingsScene.h"
#include "Examples/SphereRasterizerScenes/CubeGridScene.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Vulcant
{
    namespace Examples
    {
        void FloatingSpheresScene::generate(size_t count, float baseRadius, std::vector<SphereData>& outSpheres, std::vector<SphereInitData>& outInitData)
        {
            outSpheres.clear();
            outInitData.clear();

            outSpheres.reserve(count);
            outInitData.reserve(count);

            srand(1337);

            for (size_t i = 0; i < count; i++)
            {
                // Generate a 3D spiral / galaxy cluster
                float u     = (float)rand() / (float)RAND_MAX;
                float v     = (float)rand() / (float)RAND_MAX;
                float theta = u * 2.0f * 3.14159265f * 3.0f; // 3 turns
                float dist  = std::pow(v, 0.5f) * 6.0f + 0.2f;

                float x = dist * std::cos(theta) + ((float)rand() / RAND_MAX - 0.5f) * 0.4f;
                float z = dist * std::sin(theta) + ((float)rand() / RAND_MAX - 0.5f) * 0.4f;
                float y = ((float)rand() / RAND_MAX - 0.5f) * (1.2f / (dist * 0.3f + 0.5f));

                glm::vec3 pos(x, y, z);

                // Orbit speed inversely proportional to sqrt of distance
                float speed = (0.5f + 0.5f * ((float)rand() / RAND_MAX)) * (1.5f / std::sqrt(dist));

                // Color palette based on distance and angle (vibrant nebula gradient)
                float hue = std::fmod(theta * 0.15f + dist * 0.2f, 1.0f);
                float r   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 0.0f / 3.0f));
                float g   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 1.0f / 3.0f));
                float b   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 2.0f / 3.0f));

                uint32_t cr          = (uint32_t)(std::clamp(r, 0.0f, 1.0f) * 255.0f);
                uint32_t cg          = (uint32_t)(std::clamp(g, 0.0f, 1.0f) * 255.0f);
                uint32_t cb          = (uint32_t)(std::clamp(b, 0.0f, 1.0f) * 255.0f);
                uint32_t ca          = 255;
                uint32_t packedColor = cr | (cg << 8) | (cb << 16) | (ca << 24);

                float radiusScale = 0.5f + 0.8f * ((float)rand() / RAND_MAX);
                float sphereRad   = baseRadius * radiusScale;

                SphereData sd;
                sd.center      = pos;
                sd.radius      = sphereRad;
                sd.colorPacked = packedColor;
                outSpheres.push_back(sd);

                SphereInitData init;
                init.basePosition = pos;
                init.orbitSpeed   = speed;
                init.orbitRadius  = dist;
                init.baseTheta    = theta;
                init.radiusScale  = radiusScale;
                init.packedColor  = packedColor;
                outInitData.push_back(init);
            }
        }
    }
}

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>

TEST_CASE("SphereRasterizerScenes Generation", "[SphereRasterizerScenes]")
{
    Vulcant::Examples::FloatingSpheresScene floatingScene;
    Vulcant::Examples::CubeGridScene        gridScene;
    Vulcant::Examples::ConcentricRingsScene ringScene;

    REQUIRE(floatingScene.getName() == "Floating Spheres");
    REQUIRE(gridScene.getName() == "3D Cube Grid");
    REQUIRE(ringScene.getName() == "Concentric Rings");

    std::vector<SphereData>     spheres;
    std::vector<SphereInitData> initData;

    floatingScene.generate(100, 0.05f, spheres, initData);
    REQUIRE(spheres.size() == 100);
    REQUIRE(initData.size() == 100);

    gridScene.generate(100, 0.05f, spheres, initData);
    REQUIRE(spheres.size() == 100);
    REQUIRE(initData.size() == 100);

    ringScene.generate(100, 0.05f, spheres, initData);
    REQUIRE(spheres.size() == 100);
    REQUIRE(initData.size() == 100);
}
#endif
