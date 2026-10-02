#include "Examples/SphereRasterizerScenes/CubeGridScene.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Vulcant
{
    namespace Examples
    {
        void CubeGridScene::generate(size_t count, float baseRadius, std::vector<SphereData>& outSpheres, std::vector<SphereInitData>& outInitData)
        {
            outSpheres.clear();
            outInitData.clear();

            outSpheres.reserve(count);
            outInitData.reserve(count);

            srand(42);

            size_t dimX = (size_t)std::ceil(std::cbrt((double)count));
            size_t dimY = dimX;
            size_t dimZ = dimX;

            float spacing = 0.4f;
            float offsetX = (dimX - 1) * spacing * 0.5f;
            float offsetY = (dimY - 1) * spacing * 0.5f;
            float offsetZ = (dimZ - 1) * spacing * 0.5f;

            size_t generated = 0;
            for (size_t x = 0; x < dimX && generated < count; ++x)
            {
                for (size_t y = 0; y < dimY && generated < count; ++y)
                {
                    for (size_t z = 0; z < dimZ && generated < count; ++z, ++generated)
                    {
                        float posX = x * spacing - offsetX;
                        float posY = y * spacing - offsetY;
                        float posZ = z * spacing - offsetZ;

                        glm::vec3 pos(posX, posY, posZ);

                        float dist = glm::length(pos) + 0.1f;
                        float theta = std::atan2(posZ, posX);
                        float speed = 0.8f + 0.4f * std::sin(posX * 0.5f + posY * 0.5f);

                        float r = 0.5f + 0.5f * std::sin(posX * 0.3f);
                        float g = 0.5f + 0.5f * std::sin(posY * 0.3f + 2.094f);
                        float b = 0.5f + 0.5f * std::sin(posZ * 0.3f + 4.188f);

                        uint32_t cr = (uint32_t)(std::clamp(r, 0.0f, 1.0f) * 255.0f);
                        uint32_t cg = (uint32_t)(std::clamp(g, 0.0f, 1.0f) * 255.0f);
                        uint32_t cb = (uint32_t)(std::clamp(b, 0.0f, 1.0f) * 255.0f);
                        uint32_t ca = 255;
                        uint32_t packedColor = cr | (cg << 8) | (cb << 16) | (ca << 24);

                        float radiusScale = 0.6f + 0.4f * (float)(x + y + z) / (dimX + dimY + dimZ);
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
    }
}
