#include "VulcantVComputeCommand.h"

#include "Library/Vulcant/Wrapper/VulcanBuffer.h"
#include "Library/Vulcant/Wrapper/VulcanComputeCommand.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanSet.h"
#include "Library/Vulcant/Wrapper/VulcanShader.h"
#include "Library/Vulcant/VulcantV/VulcantVImage.h"
#include "Library/Vulcant/VulcantV/VulcantVSet.h"
#include "Library/Vulcant/VulcantV/VulcantVShader.h"

namespace Vulcant::VulcantV
{
    VulcantVComputeCommand::VulcantVComputeCommand(Wrapper::VulcanPool& pool, Wrapper::VulcanDevice& device)
    {
        cmd = std::make_unique<Wrapper::VulcanComputeCommand>(pool, device);
    }
    VulcantVComputeCommand::~VulcantVComputeCommand() {}

    void VulcantVComputeCommand::runSync()
    {
        cmd->runSync();
    }
    void VulcantVComputeCommand::runAsync()
    {
        cmd->runAsync();
    }
    void VulcantVComputeCommand::wait()
    {
        cmd->wait();
    }
    void VulcantVComputeCommand::startRecord()
    {
        cmd->startRecord();
    }
    void VulcantVComputeCommand::endRecord()
    {
        cmd->endRecord();
    }

    void VulcantVComputeCommand::add(const glm::ivec3& groupCount, VulcantSet& setInput, VulcantShader& shaderInput)
    {
        auto& set    = (VulcantVSet&)setInput;
        auto& shader = (VulcantVShader&)shaderInput;
        cmd->add(groupCount, *set.set, *shader.shader);
    }

    void VulcantVComputeCommand::addBarrier(VulcantBuffer& buffer)
    {
        cmd->addBarrier((Wrapper::VulcanBuffer&)buffer);
    }

    void VulcantVComputeCommand::addBarrier(VulcantImage& img, const VulcantResourceLayout& dest)
    {
        cmd->addBarrier(*((VulcantVImage&)img).img, dest);
    }

    void VulcantVComputeCommand::addCopyBuffer(VulcantBuffer& source, VulcantBuffer& dest, size_t elementCount, size_t sourceOffset, size_t destOffset)
    {
        cmd->addCopyBuffer((Wrapper::VulcanBuffer&)source, (Wrapper::VulcanBuffer&)dest, elementCount, sourceOffset, destOffset);
    }

    void VulcantVComputeCommand::uploadImageToGPU(VulcantImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset)
    {
        cmd->uploadImageToGPU(*((VulcantVImage&)destImage).img, cpuData, extent, offset);
    }

    void VulcantVComputeCommand::downloadImageFromGPU(VulcantImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)
    {
        cmd->uploadImageToGPU(*((VulcantVImage&)srcImage).img, outData, extent, offset);
    }
}
