#include "VulcantVComputeCommand.h"

#include "Vulcant/Wrapper/VulcanBuffer.h"
#include "Vulcant/Wrapper/VulcanComputeCommand.h"
#include "Vulcant/Wrapper/VulcanImage.h"
#include "Vulcant/Wrapper/VulcanSet.h"
#include "Vulcant/Wrapper/VulcanShader.h"
#include "Vulcant/VulcantV/VulcantVImage.h"
#include "Vulcant/VulcantV/VulcantVSet.h"
#include "Vulcant/VulcantV/VulcantVBuffer.h"
#include "Vulcant/VulcantV/VulcantVShader.h"

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
        cmd->addBarrier(*static_cast<VulcantVBuffer&>(buffer).buffer);
    }

    void VulcantVComputeCommand::addBarrier(VulcantImage& img, const VulcantResourceLayout& dest)
    {
        cmd->addBarrier(*((VulcantVImage&)img).img, dest);
    }

    void VulcantVComputeCommand::addCopyBuffer(VulcantBuffer& source, VulcantBuffer& dest, size_t elementCount, size_t sourceOffset, size_t destOffset)
    {
        cmd->addCopyBuffer(*static_cast<VulcantVBuffer&>(source).buffer, *static_cast<VulcantVBuffer&>(dest).buffer, elementCount, sourceOffset, destOffset);
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

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>
#include "VulcantVDevice.h"
#include "VulcantVShader.h"
#include "VulcantVResource.h"

TEST_CASE("VulcantVComputeCommand Execution and Compute Pipeline Dispatch", "[VulcantVComputeCommand]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    std::string glslShader = R"(
        #version 450
        layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
        layout(std430, set = 0, binding = 0) buffer PosBuffer {
            uint vals[];
        };
        void main() {
            uint idx = gl_GlobalInvocationID.x;
            vals[idx] = vals[idx] * 5 + 1;
        }
    )";

    auto shader = device.createShader(glslShader);
    REQUIRE(shader != nullptr);

    const size_t count = 64;
    auto buffer = device.createBuffer(count, sizeof(uint32_t), false);

    std::vector<uint32_t> initialVals(count);
    for (size_t i = 0; i < count; ++i) {
        initialVals[i] = static_cast<uint32_t>(i);
    }
    buffer->uploadToGPU(initialVals.data(), count, 0);

    std::shared_ptr<Vulcant::VulcantResource> res = buffer->asResource();
    auto set = device.createSet({{ res }}, *shader);

    auto cmd = device.createComputeCommand();
    cmd->startRecord();
    cmd->add(glm::ivec3(1, 1, 1), *set, *shader);
    cmd->addBarrier(*buffer);
    cmd->endRecord();

    cmd->runSync();

    std::vector<uint32_t> resultVals(count, 0);
    buffer->downloadFromGPU(resultVals.data(), count, 0);

    for (size_t i = 0; i < count; ++i) {
        REQUIRE(resultVals[i] == initialVals[i] * 5 + 1);
    }
}

TEST_CASE("VulcantVComputeCommand Buffer Copy", "[VulcantVComputeCommand]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    const size_t count = 32;
    auto srcBuffer = device.createBuffer(count, sizeof(uint32_t), false);
    auto dstBuffer = device.createBuffer(count, sizeof(uint32_t), false);

    std::vector<uint32_t> srcData(count);
    for (size_t i = 0; i < count; ++i) {
        srcData[i] = static_cast<uint32_t>(100 + i);
    }
    srcBuffer->uploadToGPU(srcData.data(), count, 0);

    auto cmd = device.createComputeCommand();
    cmd->startRecord();
    cmd->addCopyBuffer(*srcBuffer, *dstBuffer, count, 0, 0);
    cmd->endRecord();

    cmd->runSync();

    std::vector<uint32_t> dstData(count, 0);
    dstBuffer->downloadFromGPU(dstData.data(), count, 0);

    REQUIRE(srcData == dstData);
}
#endif
