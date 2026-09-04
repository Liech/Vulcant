#include "VulcantGComputeCommand.h"

#include "VulcantG/Wrapper/VulcanComputeCommand.h"
#include "VulcantG/Wrapper/VulcanShader.h"
#include "VulcantG/Wrapper/VulcanSet.h"
#include "VulcantG/Wrapper/VulcanImage.h"
#include "VulcantGImage.h"
#include "VulcantGSet.h"
#include "VulcantGShader.h"

namespace Vulcant::VulcantG
{
    VulcantGComputeCommand::VulcantGComputeCommand(Wrapper::VulcanDevice& device)
    {
        cmd = std::make_unique<Wrapper::VulcanComputeCommand>(device);
    }
    VulcantGComputeCommand::~VulcantGComputeCommand() {}

    void VulcantGComputeCommand::runSync()
    {
        cmd->runSync();
    }
    void VulcantGComputeCommand::runAsync()
    {
        cmd->runAsync();
    }
    void VulcantGComputeCommand::wait()
    {
        cmd->wait();
    }
    void VulcantGComputeCommand::startRecord()
    {
        cmd->startRecord();
    }
    void VulcantGComputeCommand::endRecord()
    {
        cmd->endRecord();
    }

    void VulcantGComputeCommand::add(const glm::ivec3& groupCount, VulcantSet& setInput, VulcantShader& shaderInput)
    {
        auto& set    = (VulcantGSet&)setInput;
        auto& shader = (VulcantGShader&)shaderInput;
        godot::Vector3i groupCountGD = godot::Vector3i(groupCount.x, groupCount.y, groupCount.z);
        cmd->add(groupCountGD, *set.set, *shader.shader);
    }

    void VulcantGComputeCommand::addBarrier(VulcantBuffer& buffer)
    {
        // godot does not need this
    }

    void VulcantGComputeCommand::addBarrier(VulcantImage& img, const VulcantResourceLayout& dest)
    {
        // godot does not need this
    }

    void VulcantGComputeCommand::addCopyBuffer(VulcantBuffer& source, VulcantBuffer& dest, size_t elementCount, size_t sourceOffset, size_t destOffset)
    {
        cmd->addCopyBuffer((Wrapper::VulcanBuffer&)source, (Wrapper::VulcanBuffer&)dest, elementCount, sourceOffset, destOffset);
    }

    void VulcantGComputeCommand::uploadImageToGPU(VulcantImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset)
    {
        cmd->uploadImageToGPU(*((VulcantGImage&)destImage).img, cpuData, extent, offset);
    }

    void VulcantGComputeCommand::downloadImageFromGPU(VulcantImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)
    {
        cmd->uploadImageToGPU(*((VulcantGImage&)srcImage).img, outData, extent, offset);
    }
}