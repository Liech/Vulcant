#include "VulcanBuffer.h"
#include <godot_cpp/classes/rendering_device.hpp>

#include "VulcanDevice.h"
#include "VulcanResource.h"

namespace Vulcant::VulcantG::Wrapper
{
    VulcanBuffer::VulcanBuffer(size_t numberOfElements, size_t elementSize, VulcanDevice& device, VulcantBufferType type, bool gpuOnlyInput)
      : device(device)
      , numberOfElements(static_cast<uint32_t>(numberOfElements))
      , elementSize(static_cast<uint32_t>(elementSize))
      , gpuOnly(gpuOnlyInput)
      , bufferType(type)
    {
        createBuffer();
    }

    VulcanBuffer::VulcanBuffer(size_t numberOfElements, size_t elementSize, VulcanDevice& device, bool ssbo, bool gpuOnlyInput)
      : VulcanBuffer(numberOfElements, elementSize, device, ssbo ? VulcantBufferType::Storage : VulcantBufferType::Uniform, gpuOnlyInput)
    {
    }

    VulcanBuffer::~VulcanBuffer()
    {
        if (bufferRid.is_valid())
        {
            device.getDevice().free_rid(bufferRid);
        }
    }

    void VulcanBuffer::createBuffer()
    {
        size_t total_size = getTotalByteSize();

        switch (bufferType)
        {
            case VulcantBufferType::Storage:
                bufferRid = device.getDevice().storage_buffer_create(total_size);
                break;
            case VulcantBufferType::Uniform:
                bufferRid = device.getDevice().uniform_buffer_create(total_size);
                break;
            case VulcantBufferType::Vertex:
                bufferRid = device.getDevice().vertex_buffer_create(static_cast<uint32_t>(total_size));
                break;
            case VulcantBufferType::Index:
                bufferRid = device.getDevice().index_buffer_create(static_cast<uint32_t>(numberOfElements),
                                                                    elementSize == 2 ? godot::RenderingDevice::INDEX_BUFFER_FORMAT_UINT16 : godot::RenderingDevice::INDEX_BUFFER_FORMAT_UINT32);
                break;
        }
    }

    godot::RID VulcanBuffer::getRid() const
    {
        return bufferRid;
    }

    size_t VulcanBuffer::getNumberOfElements() const
    {
        return numberOfElements;
    }

    size_t VulcanBuffer::getElementSize() const
    {
        return elementSize;
    }

    size_t VulcanBuffer::getTotalByteSize() const
    {
        return numberOfElements * elementSize;
    }

    VulcantBufferType VulcanBuffer::getBufferType() const
    {
        return bufferType;
    }

    VulcanResource VulcanBuffer::asResource() const
    {
        godot::RenderingDevice::UniformType type = (bufferType == VulcantBufferType::Storage) ?
            godot::RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER :
            godot::RenderingDevice::UNIFORM_TYPE_UNIFORM_BUFFER;
        return { type, getRid() };
    }

    void VulcanBuffer::uploadToGPU(const void* data, size_t elementCount, size_t elementOffset)
    {
        if (gpuOnly)
            throw std::runtime_error("Can't upload to a GPU Only Buffer!");
        if (data == nullptr || elementCount == 0)
            return;

        size_t sizeInBytes   = elementCount * elementSize;
        size_t offsetInBytes = elementOffset * elementSize;

        if (offsetInBytes + sizeInBytes > getTotalByteSize())
        {
            return;
        }
        godot::PackedByteArray byte_array;
        byte_array.resize(sizeInBytes);
        uint8_t* write_ptr = byte_array.ptrw();
        std::memcpy(write_ptr, data, sizeInBytes);
        device.getDevice().buffer_update(bufferRid, offsetInBytes, sizeInBytes, byte_array);
    }

    void VulcanBuffer::downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset)
    {
        if (gpuOnly)
            throw std::runtime_error("Can't download from a GPU Only Buffer!");
        if (outData == nullptr || elementCount == 0)
            return;

        size_t                 sizeInBytes   = elementCount * elementSize;
        size_t                 offsetInBytes = elementOffset * elementSize;
        godot::PackedByteArray data_from_gpu = device.getDevice().buffer_get_data(bufferRid, offsetInBytes, sizeInBytes);
        std::memcpy(outData, data_from_gpu.ptr(), sizeInBytes);
    }
}