#pragma once

#include "VulcantResourceLayout.h"
#include <glm/glm.hpp>

namespace Vulcant
{
    class VulcantSet;
    class VulcantShader;
    class VulcantBuffer;
    class VulcantImage;

    class VulcantComputeCommand
    {
      public:
        VulcantComputeCommand()          = default;
        virtual ~VulcantComputeCommand() = default;

        virtual void runSync()                                                                                                                      = 0;
        virtual void runAsync()                                                                                                                     = 0;
        virtual void wait()                                                                                                                         = 0;
        virtual void startRecord()                                                                                                                  = 0;
        virtual void endRecord()                                                                                                                    = 0;
        virtual void add(const glm::ivec3& groupCount, VulcantSet& setInput, VulcantShader& pipelineInput)                                          = 0;
        virtual void addBarrier(VulcantBuffer&)                                                                                                     = 0;
        virtual void addBarrier(VulcantImage&, const VulcantResourceLayout& dest = Vulcant::VulcantResourceLayout::General)                         = 0;
        virtual void addCopyBuffer(VulcantBuffer& source, VulcantBuffer& dest, size_t elementCount, size_t sourceOffset = 0, size_t destOffset = 0) = 0;
        virtual void uploadImageToGPU(VulcantImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset = glm::uvec3(0, 0, 0))     = 0;
        virtual void downloadImageFromGPU(VulcantImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)                              = 0;
    };
}