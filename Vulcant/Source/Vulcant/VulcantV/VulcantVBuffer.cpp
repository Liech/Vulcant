#include "VulcantVBuffer.h"

#include "Vulcant/Wrapper/VulcanBuffer.h"
#include "Vulcant/Wrapper/VulcanResource.h"
#include "VulcantVResource.h"

namespace Vulcant::VulcantV
{
    VulcantVBuffer::VulcantVBuffer(size_t numberOfElements, size_t elementSize, Wrapper::VulcanDevice& device, VulcantBufferType type, bool gpu)
    {
        buffer = std::make_unique<Wrapper::VulcanBuffer>(numberOfElements, elementSize, device, type, gpu);
    }

    VulcantVBuffer::~VulcantVBuffer() {}

    std::unique_ptr<VulcantResource> VulcantVBuffer::asResource() const
    {
        auto result = std::make_unique<VulcantVResource>();
        result->res = buffer->asResource();
        return std::move(result);
    }

    size_t VulcantVBuffer::getNumberOfElements() const
    {
        return buffer->getNumberOfElements();
    }

    size_t VulcantVBuffer::getElementSize() const
    {
        return buffer->getElementSize();
    }

    void VulcantVBuffer::uploadToGPU(const void* data, size_t elementCount, size_t elementOffset)
    {
        buffer->uploadToGPU(data, elementCount, elementOffset);
    }

    void VulcantVBuffer::downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset)
    {
        buffer->downloadFromGPU(outData, elementCount, elementOffset);
    }
}

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>
#include "VulcantVDevice.h"

TEST_CASE("VulcantVBuffer Storage Upload and Download Round-trip", "[VulcantVBuffer]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    auto buffer = device.createBuffer(100, sizeof(uint32_t), false);
    REQUIRE(buffer != nullptr);
    REQUIRE(buffer->getNumberOfElements() == 100);
    REQUIRE(buffer->getElementSize() == sizeof(uint32_t));

    std::vector<uint32_t> inputData(100);
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = static_cast<uint32_t>(i * 3 + 1);
    }

    buffer->uploadToGPU(inputData.data(), inputData.size(), 0);

    std::vector<uint32_t> outputData(100, 0);
    buffer->downloadFromGPU(outputData.data(), outputData.size(), 0);

    REQUIRE(inputData == outputData);
}

TEST_CASE("VulcantVBuffer Vertex and Indirect Creation", "[VulcantVBuffer]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    auto vb = device.createVertexBuffer(50, sizeof(float) * 3, false);
    REQUIRE(vb != nullptr);
    REQUIRE(vb->getNumberOfElements() == 50);
    REQUIRE(vb->getElementSize() == sizeof(float) * 3);

    auto ib = device.createIndirectBuffer(10, sizeof(uint32_t) * 4, false);
    REQUIRE(ib != nullptr);
    REQUIRE(ib->getNumberOfElements() == 10);
    REQUIRE(ib->getElementSize() == sizeof(uint32_t) * 4);
}

TEST_CASE("VulcantVBuffer Uniform Upload and Download", "[VulcantVBuffer]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    auto ub = device.createUniform(1, sizeof(float) * 16);
    REQUIRE(ub != nullptr);
    REQUIRE(ub->getNumberOfElements() == 1);
    REQUIRE(ub->getElementSize() == sizeof(float) * 16);

    std::vector<float> inputData(16, 1.5f);
    ub->uploadToGPU(inputData.data(), 1, 0);

    std::vector<float> outputData(16, 0.0f);
    ub->downloadFromGPU(outputData.data(), 1, 0);

    REQUIRE(inputData == outputData);
}

TEST_CASE("VulcantVBuffer Resource Conversion", "[VulcantVBuffer]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    auto buffer = device.createBuffer(10, sizeof(uint32_t), false);
    auto resource = buffer->asResource();
    REQUIRE(resource != nullptr);
}
#endif