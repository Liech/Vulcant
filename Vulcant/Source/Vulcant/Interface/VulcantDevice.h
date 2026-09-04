#pragma once

#include "VulcantImageFormat.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Vulcant
{
    class VulcantBuffer;
    class VulcantShader;
    class VulcantImage;
    class VulcantGraphicCommand;
    class VulcantComputeCommand;
    class VulcantSet;
    class VulcantResource;
    class VulcantWindow;
    class VulcantUi;
    class VulcantGraphicPipeline;

    class VulcantDevice
    {
      public:
        virtual std::unique_ptr<VulcantBuffer>          createBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly = false)                                                = 0;
        virtual std::unique_ptr<VulcantBuffer>          createUniform(size_t numberOfElements, size_t elementSize)                                                                     = 0;
        virtual std::unique_ptr<VulcantBuffer>          createVertexBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly = false)                                          = 0;
        virtual std::unique_ptr<VulcantShader>          createShader(const std::string& shader)                                                                                        = 0;
        virtual std::unique_ptr<VulcantShader>          createShader(const std::vector<uint32_t>& spirv)                                                                               = 0;
        virtual std::unique_ptr<VulcantImage>           createImage(uint32_t width, uint32_t height, uint32_t depth = 1, VulcantImageFormat = VulcantImageFormat::R32G32B32A32_SFLOAT) = 0;
        virtual std::unique_ptr<VulcantComputeCommand>  createComputeCommand()                                                                                                         = 0;
        virtual std::unique_ptr<VulcantGraphicCommand>  createGraphicCommand()                                                                                                         = 0;
        virtual std::unique_ptr<VulcantGraphicPipeline> createVulcanGraphicPipeline(const std::vector<VulcantShader*>& shader,
                                                                                    const std::vector<VulcantImage*>&  color,
                                                                                    VulcantImage*                      depth   = nullptr,
                                                                                    VulcantImage*                      stencil = nullptr)                                              = 0;
        virtual std::unique_ptr<VulcantSet>             createSet(const std::vector<std::vector<std::shared_ptr<VulcantResource>>>& buffer, VulcantShader& shader)                     = 0;
        virtual std::unique_ptr<VulcantWindow>          createWindow(const glm::ivec2& resolution, const std::string& title)                                                           = 0;
        virtual std::unique_ptr<VulcantUi>              createUi(VulcantWindow& window)                                                                                                = 0;
        virtual VulcantComputeCommand&                  getDefaultCommand()                                                                                                            = 0;
    };
}