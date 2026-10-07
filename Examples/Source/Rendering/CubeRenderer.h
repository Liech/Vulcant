#pragma once

#include "Rendering/SceneData.h"
#include "Rendering/SphereData.h"
#include "Rendering/SphereRenderer.h"
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
    using CubeData = SphereData;

    class CubeRenderer
    {
      public:
        CubeRenderer();
        CubeRenderer(Vulcant::VulcantDevice& device, const glm::ivec2& resolution = glm::ivec2(0, 0), size_t maxCubes = 500000);
        virtual ~CubeRenderer();

        void init(Vulcant::VulcantDevice& device, const glm::ivec2& resolution, size_t maxCubes = 500000);
        void setResolution(const glm::ivec2& resolution);
        void setSceneData(const SceneData& sceneData);

        // Cube input (radius is used as edge length)
        void setCubes(const std::vector<SphereData>& cubes);
        void setCubes(const SphereData* cubes, size_t count);
        void setCubesBuffer(Vulcant::VulcantBuffer& buffer, size_t count);
        void setActiveCubeCount(size_t count);

        // Aliases for convenience / drop-in compatibility
        void setSpheres(const std::vector<SphereData>& cubes) { setCubes(cubes); }
        void setSpheres(const SphereData* cubes, size_t count) { setCubes(cubes, count); }
        void setSpheresBuffer(Vulcant::VulcantBuffer& buffer, size_t count) { setCubesBuffer(buffer, count); }
        void setActiveSphereCount(size_t count) { setActiveCubeCount(count); }

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
        Vulcant::VulcantBuffer& getCubesBuffer() const;
        Vulcant::VulcantBuffer& getSpheresBuffer() const { return getCubesBuffer(); }
        Vulcant::VulcantBuffer& getCulledCubesBuffer() const;
        Vulcant::VulcantBuffer& getIndirectDrawBuffer() const;
        size_t                  getActiveCubeCount() const;
        size_t                  getActiveSphereCount() const { return getActiveCubeCount(); }
        size_t                  getMaxCubes() const;

      private:
        void createPipeline();
        void allocateOutputTextures();

        Vulcant::VulcantDevice* device             = nullptr;
        glm::ivec2              resolution         = glm::ivec2(0, 0);
        size_t                  maxCubes           = 500000;
        size_t                  activeCubeCount    = 0;
        bool                    enableCulling      = true;
        bool                    ownsOutputTextures = true;

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
        std::unique_ptr<Vulcant::VulcantBuffer> internalCubesBuffer;
        Vulcant::VulcantBuffer*                 currentCubesBuffer = nullptr;
        std::unique_ptr<Vulcant::VulcantBuffer> culledCubesBuffer;

        // Descriptor Sets
        std::unique_ptr<Vulcant::VulcantSet> graphicSet;       // (sceneDataUbo, culledCubesBuffer)
        std::unique_ptr<Vulcant::VulcantSet> graphicDirectSet; // (sceneDataUbo, currentCubesBuffer)
        std::unique_ptr<Vulcant::VulcantSet> cullSet;          // (sceneDataUbo, currentCubesBuffer, cullParamsUbo, culledCubesBuffer, indirectDrawBuffer)

        // Output images
        std::unique_ptr<Vulcant::VulcantImage> internalColor;
        std::unique_ptr<Vulcant::VulcantImage> internalDepth;
        std::unique_ptr<Vulcant::VulcantImage> internalNormal;

        Vulcant::VulcantImage* color  = nullptr;
        Vulcant::VulcantImage* depth  = nullptr;
        Vulcant::VulcantImage* normal = nullptr;
    };
}
