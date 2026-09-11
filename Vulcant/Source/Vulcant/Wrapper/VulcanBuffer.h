#pragma once

#include "Vulcant/Interface/VulcantBufferType.h"
#include <string>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    struct VulcanResource;

    class VulcanBuffer
    {
      public:
        VulcanBuffer(size_t numberOfElements, size_t elementSize, VulcanDevice& device, VulcantBufferType bufferType = VulcantBufferType::Storage, bool gpuOnly = false);
        virtual ~VulcanBuffer();

        VkDescriptorBufferInfo getInfo() const;

        size_t getNumberOfElements() const;
        size_t getElementSize() const;

        VulcanResource asResource() const;

        void uploadToGPU(const void* data, size_t elementCount, size_t elementOffset = 0);
        void downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset = 0);

        VkBuffer& __getBuffer();

      private:
        void createBuffer();

        VulcanDevice& device;

        void*    mappedMemory = nullptr;
        uint64_t bufferSize;

        VkBuffer          buffer;
        VkDeviceMemory    bufferMemory;
        uint32_t          numberOfElements;
        uint32_t          elementSize;
        bool              gpuOnly = false;
        VulcantBufferType bufferType;
    };
}