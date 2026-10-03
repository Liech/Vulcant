#include "Examples/SphereRasterizerScenes/ConcentricRingsScene.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Vulcant
{
    namespace Examples
    {
        void ConcentricRingsScene::generate(size_t count, float baseRadius, std::vector<SphereData>& outSpheres, std::vector<SphereInitData>& outInitData)
        {
            outSpheres.clear();
            outInitData.clear();

            outSpheres.reserve(count);
            outInitData.reserve(count);

            srand(999);

            const int numRings = 12;
            for (size_t i = 0; i < count; i++)
            {
                int ringIndex = (int)(i % numRings);
                float ringRadius = 0.8f + ringIndex * 0.7f;
                float theta = ((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159265f;

                float heightVar = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.3f;
                float radialNoise = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.2f;

                float dist = ringRadius + radialNoise;
                float x = dist * std::cos(theta);
                float z = dist * std::sin(theta);
                float y = heightVar + std::sin(ringIndex * 0.5f) * 0.4f;

                glm::vec3 pos(x, y, z);

                float speed = (ringIndex % 2 == 0 ? 1.0f : -1.0f) * (1.2f / (ringRadius * 0.5f + 0.5f));

                float hue = (float)ringIndex / (float)numRings;
                float r   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 0.0f / 3.0f));
                float g   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 1.0f / 3.0f));
                float b   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 2.0f / 3.0f));

                uint32_t cr          = (uint32_t)(std::clamp(r, 0.0f, 1.0f) * 255.0f);
                uint32_t cg          = (uint32_t)(std::clamp(g, 0.0f, 1.0f) * 255.0f);
                uint32_t cb          = (uint32_t)(std::clamp(b, 0.0f, 1.0f) * 255.0f);
                uint32_t ca          = 255;
                uint32_t packedColor = cr | (cg << 8) | (cb << 16) | (ca << 24);

                float radiusScale = 0.5f + 0.7f * ((float)rand() / RAND_MAX);
                float sphereRad   = baseRadius * radiusScale;

                SphereData sd;
                sd.center = pos;
                sd.setRadiusAndColor(sphereRad, packedColor);
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
