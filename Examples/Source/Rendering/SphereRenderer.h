#pragma once

#include "Rendering/SceneData.h"
#include "Rendering/SphereData.h"
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
    class VulcantComputeCommand;
    class VulcantGraphicCommand;
    class VulcantGraphicPipeline;
}

namespace Vulcant::Rendering
{
    struct CullParams
    {
        uint32_t activeSphereCount;
        uint32_t enableCulling;
        uint32_t pad0;
        uint32_t pad1;
    };

    struct VkDrawIndirectCommandCPU
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };

    class SphereRenderer
    {
      public:
        SphereRenderer();
        SphereRenderer(Vulcant::VulcantDevice& device, const glm::ivec2& resolution, size_t maxSpheres = 500000);
        virtual ~SphereRenderer();

        void init(Vulcant::VulcantDevice& device, const glm::ivec2& resolution, size_t maxSpheres = 500000);
        void setResolution(const glm::ivec2& resolution);
        void setSceneData(const SceneData& sceneData);

        // Spheres input
        void setSpheres(const std::vector<SphereData>& spheres);
        void setSpheres(const SphereData* spheres, size_t count);
        void setSpheresBuffer(Vulcant::VulcantBuffer& buffer, size_t count);
        void setActiveSphereCount(size_t count);

        // Frustum culling
        void setCullingEnabled(bool enable);
        bool isCullingEnabled() const;
        void updateCullParams();

        // Optional custom output render targets
        void setOutputTextures(Vulcant::VulcantImage& color, Vulcant::VulcantImage& depth, Vulcant::VulcantImage& normal);

        // Descriptor sets
        void updateDescriptorSets();

        // Command recording
        void recordCull(Vulcant::VulcantComputeCommand& cmd);
        void record(Vulcant::VulcantGraphicCommand& cmd);

        // Outputs digestible by DeferredShading::setInputTextures
        Vulcant::VulcantImage& getColor() const;
        Vulcant::VulcantImage& getDepth() const;
        Vulcant::VulcantImage& getNormal() const;

        // Resource accessors
        Vulcant::VulcantBuffer& getSpheresBuffer() const;
        Vulcant::VulcantBuffer& getCulledSpheresBuffer() const;
        Vulcant::VulcantBuffer& getIndirectDrawBuffer() const;
        size_t                  getActiveSphereCount() const;
        size_t                  getMaxSpheres() const;

      private:
        void createPipeline();
        void allocateOutputTextures();

        Vulcant::VulcantDevice* device               = nullptr;
        glm::ivec2              resolution           = glm::ivec2(0, 0);
        size_t                  maxSpheres           = 500000;
        size_t                  activeSphereCount    = 0;
        bool                    enableCulling        = true;
        bool                    ownsOutputTextures   = true;

        SceneData sceneData{};

        // Shaders & Pipeline
        std::unique_ptr<Vulcant::VulcantShader>          vertShader;
        std::unique_ptr<Vulcant::VulcantShader>          fragShader;
        std::unique_ptr<Vulcant::VulcantShader>          cullShader;
        std::unique_ptr<Vulcant::VulcantGraphicPipeline> pipeline;

        // Buffers
        std::unique_ptr<Vulcant::VulcantBuffer> sceneDataUbo;
        std::unique_ptr<Vulcant::VulcantBuffer> cullParamsUbo;
        std::unique_ptr<Vulcant::VulcantBuffer> indirectDrawBuffer;
        std::unique_ptr<Vulcant::VulcantBuffer> internalSpheresBuffer;
        Vulcant::VulcantBuffer*                 currentSpheresBuffer = nullptr;
        std::unique_ptr<Vulcant::VulcantBuffer> culledSpheresBuffer;

        // Descriptor Sets
        std::unique_ptr<Vulcant::VulcantSet> graphicSet;       // (sceneDataUbo, culledSpheresBuffer)
        std::unique_ptr<Vulcant::VulcantSet> graphicDirectSet; // (sceneDataUbo, currentSpheresBuffer)
        std::unique_ptr<Vulcant::VulcantSet> cullSet;          // (sceneDataUbo, currentSpheresBuffer, cullParamsUbo, culledSpheresBuffer, indirectDrawBuffer)

        // Output images
        std::unique_ptr<Vulcant::VulcantImage> internalColor;
        std::unique_ptr<Vulcant::VulcantImage> internalDepth;
        std::unique_ptr<Vulcant::VulcantImage> internalNormal;

        Vulcant::VulcantImage* color  = nullptr;
        Vulcant::VulcantImage* depth  = nullptr;
        Vulcant::VulcantImage* normal = nullptr;
    };
}
