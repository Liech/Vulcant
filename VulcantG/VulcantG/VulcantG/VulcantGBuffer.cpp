#include "VulcantGBuffer.h"

#include "VulcantG/Wrapper/VulcanBuffer.h"
#include "VulcantG/Wrapper/VulcanResource.h"
#include "VulcantGResource.h"

namespace Vulcant::VulcantG
{
    VulcantGBuffer::VulcantGBuffer(size_t numberOfElements, size_t elementSize, Wrapper::VulcanDevice& device, VulcantBufferType type, bool gpu)
    {
        buffer = std::make_unique<Wrapper::VulcanBuffer>(numberOfElements, elementSize, device, type, gpu);
    }

    VulcantGBuffer::VulcantGBuffer(size_t numberOfElements, size_t elementSize, Wrapper::VulcanDevice& device, bool ssbo, bool gpu)
    {
        buffer = std::make_unique<Wrapper::VulcanBuffer>(numberOfElements, elementSize, device, ssbo, gpu);
    }

    VulcantGBuffer::~VulcantGBuffer() {}

    std::unique_ptr<VulcantResource> VulcantGBuffer::asResource() const
    {
        auto result = std::make_unique<VulcantGResource>();
        result->res = buffer->asResource();
        return std::move(result);
    }

    size_t VulcantGBuffer::getNumberOfElements() const
    {
        return buffer->getNumberOfElements();
    }

    size_t VulcantGBuffer::getElementSize() const
    {
        return buffer->getElementSize();
    }

    void VulcantGBuffer::uploadToGPU(const void* data, size_t elementCount, size_t elementOffset)
    {
        buffer->uploadToGPU(data, elementCount, elementOffset);
    }

    void VulcantGBuffer::downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset)
    {
        buffer->downloadFromGPU(outData, elementCount, elementOffset);
    }
}