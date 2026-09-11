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