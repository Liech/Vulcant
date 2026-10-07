#pragma once

#include "Rendering/SceneData.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantSet;
    class VulcantBuffer;
    class VulcantGraphicCommand;
    class VulcantGraphicPipeline;
}

namespace Vulcant::Rendering
{
    struct MeshVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
    };

    class MeshRenderer
    {
      public:
        MeshRenderer();
        MeshRenderer(Vulcant::VulcantDevice& device, const glm::ivec2& resolution = glm::ivec2(0, 0));
        virtual ~MeshRenderer();

        void init(Vulcant::VulcantDevice& device, const glm::ivec2& resolution);
        void setResolution(const glm::ivec2& resolution);
        void setSceneData(const SceneData& sceneData);

        // Upload mesh geometry
        void setMesh(const std::vector<glm::vec3>& positions,
                     const std::vector<uint32_t>&  indices,
                     const glm::vec3&              color = glm::vec3(0.8f, 0.8f, 0.8f));

        void setMesh(const std::vector<glm::vec3>& positions,
                     const std::vector<glm::vec3>& normals,
                     const std::vector<uint32_t>&  indices,
                     const glm::vec3&              color = glm::vec3(0.8f, 0.8f, 0.8f));

        void setVertices(const std::vector<MeshVertex>& vertices);

        // Generic support for static Triangulation (duck-typed with .vertices and .indices)
        template<typename TTriangulation>
        void setTriangulation(const TTriangulation& tri, const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f))
        {
            if (tri.indices.empty())
            {
                vertexCount = 0;
                return;
            }

            std::vector<MeshVertex> meshVertices;
            meshVertices.reserve(tri.indices.size());

            // STL/triangulation triangle soup: 3 indices per face
            for (size_t i = 0; i + 2 < tri.indices.size(); i += 3)
            {
                size_t idx0 = tri.indices[i];
                size_t idx1 = tri.indices[i + 1];
                size_t idx2 = tri.indices[i + 2];

                if (idx0 >= tri.vertices.size() || idx1 >= tri.vertices.size() || idx2 >= tri.vertices.size())
                    continue;

                glm::vec3 p0 = glm::vec3(tri.vertices[idx0]);
                glm::vec3 p1 = glm::vec3(tri.vertices[idx1]);
                glm::vec3 p2 = glm::vec3(tri.vertices[idx2]);

                glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);
                float     len    = glm::length(normal);
                if (len > 1e-6f)
                {
                    normal /= len;
                }
                else
                {
                    normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                meshVertices.push_back({ p0, normal, color });
                meshVertices.push_back({ p1, normal, color });
                meshVertices.push_back({ p2, normal, color });
            }

            setVertices(meshVertices);
        }

        // Custom render targets (e.g. for DeferredShading G-Buffer integration)
        void setOutputTextures(Vulcant::VulcantImage& color, Vulcant::VulcantImage& depth, Vulcant::VulcantImage& normal);

        // Descriptor sets & recording
        void updateDescriptorSets();
        void record(Vulcant::VulcantGraphicCommand& cmd);

        // Texture accessors
        Vulcant::VulcantImage& getColor() const;
        Vulcant::VulcantImage& getDepth() const;
        Vulcant::VulcantImage& getNormal() const;

        uint32_t getVertexCount() const { return vertexCount; }
        uint32_t getTriangleCount() const { return vertexCount / 3; }

      private:
        void createPipeline();
        void allocateOutputTextures();

        Vulcant::VulcantDevice* device             = nullptr;
        glm::ivec2              resolution         = glm::ivec2(0, 0);
        uint32_t                vertexCount        = 0;
        bool                    ownsOutputTextures = true;

        SceneData sceneData{};

        // Shaders & Pipeline
        std::unique_ptr<Vulcant::VulcantShader>          vertShader;
        std::unique_ptr<Vulcant::VulcantShader>          fragShader;
        std::unique_ptr<Vulcant::VulcantGraphicPipeline> pipeline;

        // Buffers
        std::unique_ptr<Vulcant::VulcantBuffer> sceneDataUbo;
        std::unique_ptr<Vulcant::VulcantBuffer> vertexBuffer;

        // Descriptor Set
        std::unique_ptr<Vulcant::VulcantSet> graphicSet;

        // Output images
        std::unique_ptr<Vulcant::VulcantImage> internalColor;
        std::unique_ptr<Vulcant::VulcantImage> internalDepth;
        std::unique_ptr<Vulcant::VulcantImage> internalNormal;

        Vulcant::VulcantImage* color  = nullptr;
        Vulcant::VulcantImage* depth  = nullptr;
        Vulcant::VulcantImage* normal = nullptr;
    };
}
