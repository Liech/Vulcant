#pragma once

#include "Vulcant/Interface/VulcantBufferType.h"
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <string>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;
    struct VulcanResource;

    class VulcanBuffer
    {
      public:
        VulcanBuffer(size_t numberOfElements, size_t elementSize, VulcanDevice& device, VulcantBufferType bufferType = VulcantBufferType::Storage, bool gpuOnly = false);
        VulcanBuffer(size_t numberOfElements, size_t elementSize, VulcanDevice& device, bool ssbo, bool gpuOnly = false);
        virtual ~VulcanBuffer();

        godot::RID getRid() const;

        size_t getNumberOfElements() const;
        size_t getElementSize() const;
        size_t getTotalByteSize() const;
        VulcantBufferType getBufferType() const;

        VulcanResource asResource() const;

        void uploadToGPU(const void* data, size_t elementCount, size_t elementOffset = 0);
        void downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset = 0);

      private:
        void createBuffer();

        VulcanDevice& device;

        godot::RID bufferRid;
        uint32_t   numberOfElements;
        uint32_t   elementSize;
        bool       gpuOnly = false;
        VulcantBufferType bufferType = VulcantBufferType::Storage;
    };
}