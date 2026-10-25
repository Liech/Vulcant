#pragma once

#include "Vulcant/Interface/VulcantCullMode.h"
#include "Vulcant/Toolkit/MeshShaderParams.h"
#include "Vulcant/Toolkit/SceneData.h"

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

namespace Vulcant::Toolkit
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
        enum class WireframeMode
        {
            Off           = 0,
            Overlay       = 1,
            WireframeOnly = 2
        };

        MeshRenderer();
        MeshRenderer(Vulcant::VulcantDevice& device, const glm::ivec2& resolution = glm::ivec2(0, 0));
        virtual ~MeshRenderer();

        void              init(Vulcant::VulcantDevice& device, const glm::ivec2& resolution);
        void              setResolution(const glm::ivec2& resolution);
        const glm::ivec2& getResolution() const;
        void              setSceneData(const SceneData& sceneData);

        void             setWireframeMode(WireframeMode mode);
        void             setWireframeColor(const glm::vec3& color);
        WireframeMode    getWireframeMode() const;
        const glm::vec3& getWireframeColor() const;

        void                     setCullMode(Vulcant::VulcantCullMode mode);
        Vulcant::VulcantCullMode getCullMode() const;

        void setClear(bool clear);
        bool isClear() const;

        void setMesh(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f));
        void setMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<uint32_t>& indices, const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f));
        void setVertices(const std::vector<MeshVertex>& vertices);

        template<typename TTriangulation>
        void setTriangulation(const TTriangulation& tri, const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f));

        void setOutputTextures(Vulcant::VulcantImage& color, Vulcant::VulcantImage& depth, Vulcant::VulcantImage& normal);

        void updateDescriptorSets();
        void record(Vulcant::VulcantGraphicCommand& cmd);
        void record(Vulcant::VulcantGraphicCommand& cmd, Vulcant::VulcantBuffer& customVertexBuffer, uint32_t customVertexCount);

        Vulcant::VulcantImage& getColor() const;
        Vulcant::VulcantImage& getDepth() const;
        Vulcant::VulcantImage& getNormal() const;

        uint32_t getVertexCount() const;
        uint32_t getTriangleCount() const;

      private:
        void createPipeline();
        void allocateOutputTextures();
        void updateMeshParamsBuffer();

        Vulcant::VulcantDevice*  device             = nullptr;
        glm::ivec2               resolution         = glm::ivec2(0, 0);
        uint32_t                 vertexCount        = 0;
        bool                     ownsOutputTextures = true;
        WireframeMode            wireframeMode      = WireframeMode::Off;
        glm::vec3                wireframeColor     = glm::vec3(1.0f, 0.0f, 0.0f);
        Vulcant::VulcantCullMode cullMode           = Vulcant::VulcantCullMode::None;
        bool                     clear              = true;

        SceneData        sceneData{};
        MeshShaderParams meshParams{};

        std::unique_ptr<Vulcant::VulcantShader>          vertShader;
        std::unique_ptr<Vulcant::VulcantShader>          fragShader;
        std::unique_ptr<Vulcant::VulcantGraphicPipeline> pipeline;

        std::unique_ptr<Vulcant::VulcantBuffer> sceneDataUbo;
        std::unique_ptr<Vulcant::VulcantBuffer> meshParamsUbo;
        std::unique_ptr<Vulcant::VulcantBuffer> vertexBuffer;
        std::unique_ptr<Vulcant::VulcantBuffer> indirectBuffer;

        std::unique_ptr<Vulcant::VulcantSet> graphicSet;

        std::unique_ptr<Vulcant::VulcantImage> internalColor;
        std::unique_ptr<Vulcant::VulcantImage> internalDepth;
        std::unique_ptr<Vulcant::VulcantImage> internalNormal;

        Vulcant::VulcantImage* color  = nullptr;
        Vulcant::VulcantImage* depth  = nullptr;
        Vulcant::VulcantImage* normal = nullptr;
    };

    template<typename TTriangulation>
    void MeshRenderer::setTriangulation(const TTriangulation& tri, const glm::vec3& color)
    {
        if (tri.indices.empty())
        {
            vertexCount = 0;
            return;
        }

        std::vector<MeshVertex> meshVertices;
        meshVertices.reserve(tri.indices.size());

        for (size_t i = 0; i + 2 < tri.indices.size(); i += 3)
        {
            size_t idx0 = tri.indices[i];
            size_t idx1 = tri.indices[i + 1];
            size_t idx2 = tri.indices[i + 2];

            if (idx0 >= tri.vertices.size() || idx1 >= tri.vertices.size() || idx2 >= tri.vertices.size())
            {
                continue;
            }

            glm::vec3 p0 = glm::vec3(tri.vertices[idx0]);
            glm::vec3 p1 = glm::vec3(tri.vertices[idx1]);
            glm::vec3 p2 = glm::vec3(tri.vertices[idx2]);

            glm::vec3 normalVal = glm::cross(p1 - p0, p2 - p0);
            float     len       = glm::length(normalVal);
            if (len > 1e-6f)
            {
                normalVal /= len;
            }
            else
            {
                normalVal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            meshVertices.push_back({ p0, normalVal, color });
            meshVertices.push_back({ p1, normalVal, color });
            meshVertices.push_back({ p2, normalVal, color });
        }

        setVertices(meshVertices);
    }
}
