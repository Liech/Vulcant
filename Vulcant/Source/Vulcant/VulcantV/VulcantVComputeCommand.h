#pragma once

#include "Library/Vulcant/Interface/VulcantComputeCommand.h"
#include <memory>

namespace Vulcant::Wrapper
{
    class VulcanComputeCommand;
    class VulcanPool;
    class VulcanDevice;
    class VulcanShader;
    class VulcanSet;
}

namespace Vulcant::VulcantV
{
    class VulcantVComputeCommand : public VulcantComputeCommand
    {
      public:
        VulcantVComputeCommand(Wrapper::VulcanPool& pool, Wrapper::VulcanDevice& device);
        virtual ~VulcantVComputeCommand();

        virtual void runSync() override;
        virtual void runAsync() override;
        virtual void wait() override;
        virtual void startRecord() override;
        virtual void endRecord() override;
        virtual void add(const glm::ivec3& groupCount, VulcantSet& set, VulcantShader& shader) override;
        virtual void addBarrier(VulcantBuffer&) override;
        virtual void addBarrier(VulcantImage&, const VulcantResourceLayout& dest) override;
        virtual void addCopyBuffer(VulcantBuffer& source, VulcantBuffer& dest, size_t elementCount, size_t sourceOffset = 0, size_t destOffset = 0) override;
        void         uploadImageToGPU(VulcantImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset = glm::uvec3(0, 0, 0)) override;
        void         downloadImageFromGPU(VulcantImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset) override;


        std::unique_ptr<Wrapper::VulcanComputeCommand> cmd;
    };
}
