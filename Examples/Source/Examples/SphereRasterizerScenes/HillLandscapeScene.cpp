#include "Examples/SphereRasterizerScenes/HillLandscapeScene.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Vulcant
{
    namespace Examples
    {
        static float getTerrainHeight(float x, float z)
        {
            // Non-flat rolling hill elevation formula using multiple sine/cosine frequencies
            float h = 0.0f;
            h += 8.0f * std::sin(x * 0.015f) * std::cos(z * 0.015f);
            h += 4.0f * std::sin(x * 0.035f + 1.2f) * std::sin(z * 0.03f + 2.1f);
            h += 1.5f * std::cos(x * 0.08f - z * 0.07f);
            h += 0.5f * std::sin(x * 0.2f + z * 0.15f);
            return h;
        }

        static uint32_t packRGBA(float r, float g, float b, float a = 1.0f)
        {
            uint32_t cr = (uint32_t)(std::clamp(r, 0.0f, 1.0f) * 255.0f);
            uint32_t cg = (uint32_t)(std::clamp(g, 0.0f, 1.0f) * 255.0f);
            uint32_t cb = (uint32_t)(std::clamp(b, 0.0f, 1.0f) * 255.0f);
            uint32_t ca = (uint32_t)(std::clamp(a, 0.0f, 1.0f) * 255.0f);
            return cr | (cg << 8) | (cb << 16) | (ca << 24);
        }

        void HillLandscapeScene::generate(size_t count, float baseRadius, std::vector<SphereData>& outSpheres, std::vector<SphereInitData>& outInitData)
        {
            outSpheres.clear();
            outInitData.clear();

            outSpheres.reserve(count);
            outInitData.reserve(count);

            srand(2026);

            // Allocate ~80% of spheres to landscape terrain and ~20% to trees
            size_t treeCountGoal = (count >= 20) ? count / 5 : 0;
            size_t terrainCountGoal = count - treeCountGoal;

            const float maxRadius = 1000.0f; // Huge distance requirement

            // 1. Generate Terrain Spheres
            for (size_t i = 0; i < terrainCountGoal && outSpheres.size() < count; ++i)
            {
                // Golden angle spiral distribution with non-linear radial scaling for huge extent
                float fraction = (float)i / (float)(terrainCountGoal > 0 ? terrainCountGoal : 1);
                float r = std::pow(fraction, 1.35f) * maxRadius + 0.1f;
                float phi = (float)i * 2.399963229728653f; // Golden angle in radians

                float x = r * std::cos(phi);
                float z = r * std::sin(phi);
                float y = getTerrainHeight(x, z);

                // Grass color variation based on altitude and position
                float altFactor = std::clamp((y + 10.0f) / 25.0f, 0.0f, 1.0f);
                float noise = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f;

                float red = 0.15f + altFactor * 0.25f + noise;
                float green = 0.45f + altFactor * 0.35f + noise;
                float blue = 0.10f + altFactor * 0.15f;

                uint32_t packedColor = packRGBA(red, green, blue);

                // Scale radius slightly with distance to ensure coverage at huge distances
                float distScale = 1.0f + (r / maxRadius) * 2.5f;
                float radiusScale = (0.7f + 0.5f * ((float)rand() / (float)RAND_MAX)) * distScale;
                float sphereRad = baseRadius * radiusScale;

                glm::vec3 pos(x, y, z);
                float theta = std::atan2(z, x);

                SphereData sd;
                sd.center = pos;
                sd.radius = sphereRad;
                sd.colorPacked = packedColor;
                outSpheres.push_back(sd);

                SphereInitData init;
                init.basePosition = pos;
                init.orbitSpeed = 0.0f; // Static landscape
                init.orbitRadius = r;
                init.baseTheta = theta;
                init.radiusScale = radiusScale;
                init.packedColor = packedColor;
                outInitData.push_back(init);
            }

            // 2. Generate Trees (Trunk stacks + Foliage crowns)
            size_t generatedSoFar = outSpheres.size();
            size_t remainingSpheres = (count > generatedSoFar) ? count - generatedSoFar : 0;

            if (remainingSpheres > 0)
            {
                // Each tree will consume ~8 spheres (2 trunk, 6 foliage)
                size_t spheresPerTree = 8;
                size_t numTrees = std::max<size_t>(1, remainingSpheres / spheresPerTree);

                for (size_t t = 0; t < numTrees && outSpheres.size() < count; ++t)
                {
                    // Tree distribution radius across landscape up to 850 units
                    float treeFrac = (float)t / (float)numTrees;
                    float tr = std::pow(treeFrac, 1.2f) * (maxRadius * 0.85f) + 1.0f;
                    float tphi = (float)t * 3.8196601125f + ((float)rand() / (float)RAND_MAX);

                    float tx = tr * std::cos(tphi);
                    float tz = tr * std::sin(tphi);
                    float ty = getTerrainHeight(tx, tz);

                    float distScale = 1.0f + (tr / maxRadius) * 2.0f;

                    // Trunk (2 brown spheres stacked)
                    float trunkHeight = 1.2f * distScale;
                    for (int k = 0; k < 2 && outSpheres.size() < count; ++k)
                    {
                        float py = ty + (k + 0.5f) * (trunkHeight * 0.5f);
                        glm::vec3 pos(tx, py, tz);

                        float rScale = 0.8f * distScale;
                        uint32_t trunkColor = packRGBA(0.40f, 0.25f, 0.12f); // Brown trunk

                        float dist = std::sqrt(tx * tx + tz * tz);
                        float theta = std::atan2(tz, tx);

                        SphereData sd;
                        sd.center = pos;
                        sd.radius = baseRadius * rScale;
                        sd.colorPacked = trunkColor;
                        outSpheres.push_back(sd);

                        SphereInitData init;
                        init.basePosition = pos;
                        init.orbitSpeed = 0.0f;
                        init.orbitRadius = dist;
                        init.baseTheta = theta;
                        init.radiusScale = rScale;
                        init.packedColor = trunkColor;
                        outInitData.push_back(init);
                    }

                    // Foliage Crown (cluster of green spheres at top of trunk)
                    float crownBaseY = ty + trunkHeight;
                    float crownRadius = 1.5f * distScale;

                    // Foliage color variation
                    float fHue = ((float)rand() / (float)RAND_MAX) * 0.15f;
                    uint32_t foliageColor = packRGBA(0.08f + fHue, 0.45f + fHue * 1.2f, 0.12f);

                    // 6 foliage spheres around the crown center
                    for (int f = 0; f < 6 && outSpheres.size() < count; ++f)
                    {
                        float angle = f * (6.2831853f / 6.0f);
                        float fRadiusOffset = (f == 0) ? 0.0f : crownRadius * 0.5f;
                        float fx = tx + fRadiusOffset * std::cos(angle);
                        float fz = tz + fRadiusOffset * std::sin(angle);
                        float fy = crownBaseY + ((f % 2 == 0) ? 0.3f : 0.8f) * distScale;

                        glm::vec3 pos(fx, fy, fz);
                        float rScale = (1.2f + 0.3f * (f % 3)) * distScale;

                        float dist = std::sqrt(fx * fx + fz * fz);
                        float theta = std::atan2(fz, fx);

                        SphereData sd;
                        sd.center = pos;
                        sd.radius = baseRadius * rScale;
                        sd.colorPacked = foliageColor;
                        outSpheres.push_back(sd);

                        SphereInitData init;
                        init.basePosition = pos;
                        init.orbitSpeed = 0.0f;
                        init.orbitRadius = dist;
                        init.baseTheta = theta;
                        init.radiusScale = rScale;
                        init.packedColor = foliageColor;
                        outInitData.push_back(init);
                    }
                }
            }

            // Fill any remaining required count if count wasn't exactly hit
            while (outSpheres.size() < count)
            {
                size_t idx = outSpheres.size();
                float fraction = (float)idx / (float)count;
                float r = std::pow(fraction, 1.3f) * maxRadius + 0.1f;
                float phi = (float)idx * 2.399963229728653f;

                float x = r * std::cos(phi);
                float z = r * std::sin(phi);
                float y = getTerrainHeight(x, z);

                uint32_t color = packRGBA(0.2f, 0.5f, 0.2f);
                float distScale = 1.0f + (r / maxRadius) * 2.5f;
                float rScale = 1.0f * distScale;

                glm::vec3 pos(x, y, z);
                float theta = std::atan2(z, x);

                SphereData sd{ pos, baseRadius * rScale, color };
                outSpheres.push_back(sd);

                SphereInitData init{ pos, 0.0f, r, theta, rScale, color };
                outInitData.push_back(init);
            }
        }
    }
}
