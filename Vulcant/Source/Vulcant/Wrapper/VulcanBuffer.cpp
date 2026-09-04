#include "VulcanBuffer.h"

#include "VulcanDevice.h"
#include "VulcanInstance.h"
#include "VulcanResource.h"
#include "lodepng.h"
#include <stdexcept>

namespace Vulcant::Wrapper
{
    VulcanBuffer::VulcanBuffer(size_t numberOfElementsInput, size_t elementSizeInput, VulcanDevice& deviceInput, VulcantBufferType bufferTypeInput, bool gpuOnlyInput)
      : device(deviceInput)
    {
        numberOfElements = numberOfElementsInput;
        elementSize      = elementSizeInput;
        bufferSize       = numberOfElements * elementSize;
        gpuOnly          = gpuOnlyInput;
        bufferType       = bufferTypeInput;
        createBuffer();
    }
    VulcanBuffer::~VulcanBuffer()
    {
        if (mappedMemory)
        {
            vkUnmapMemory(device.getDevice(), bufferMemory);
        }
        vkFreeMemory(device.getDevice(), bufferMemory, NULL);
        vkDestroyBuffer(device.getDevice(), buffer, NULL);
    }

    size_t VulcanBuffer::getNumberOfElements() const
    {
        return numberOfElements;
    }

    size_t VulcanBuffer::getElementSize() const
    {
        return elementSize;
    }

    VkDescriptorBufferInfo VulcanBuffer::getInfo() const
    {
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer                 = buffer;
        descriptorBufferInfo.offset                 = 0;
        descriptorBufferInfo.range                  = numberOfElements * elementSize;
        return descriptorBufferInfo;
    }

    void VulcanBuffer::createBuffer()
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size               = bufferSize;

        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        switch (bufferType)
        {
            case VulcantBufferType::Storage:
                bufferCreateInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                break;
            case VulcantBufferType::Uniform:
                bufferCreateInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                break;
            case VulcantBufferType::Vertex:
                bufferCreateInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                break;
            case VulcantBufferType::Index:
                bufferCreateInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                break;
        }

        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device.getDevice(), &bufferCreateInfo, NULL, &buffer));

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memReqs);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize       = memReqs.size;

        if (gpuOnly)
        {
            // Rein auf der Grafikkarte
            allocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
        else
        {
            // Für CPU Zugriff (Staging)
            allocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        VK_CHECK_RESULT(vkAllocateMemory(device.getDevice(), &allocInfo, NULL, &bufferMemory));
        vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);

        // Nur mappen, wenn nicht gpuOnly
        if (!gpuOnly)
        {
            vkMapMemory(device.getDevice(), bufferMemory, 0, bufferSize, 0, &mappedMemory);
        }
        else
        {
            mappedMemory = nullptr;
        }
    }

    VulcanResource VulcanBuffer::asResource() const
    {
        VkDescriptorType type;
        switch (bufferType)
        {
            case VulcantBufferType::Storage:
                type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case VulcantBufferType::Uniform:
                type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case VulcantBufferType::Vertex:
                throw std::runtime_error("Vertex buffers cannot be used as resources!");
                break;
            case VulcantBufferType::Index:
                throw std::runtime_error("Index buffers cannot be used as resources!");
                break;
            default:
                throw std::runtime_error("Unkown Buffer Type!");
        }
        return { type, getInfo(), {} };
    }

    void VulcanBuffer::uploadToGPU(const void* data, size_t elementCount, size_t elementOffset)
    {
        if (gpuOnly)
            throw std::runtime_error("Can't upload to a GPU Only Buffer!");

        // Calculate byte offset and size
        size_t sizeInBytes   = elementCount * elementSize;
        size_t offsetInBytes = elementOffset * elementSize;

        // Directly copy to the mapped memory
        std::memcpy(static_cast<uint8_t*>(mappedMemory) + offsetInBytes, data, sizeInBytes);
    }

    void VulcanBuffer::downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset)
    {
        if (gpuOnly)
            throw std::runtime_error("Can't download from a GPU Only Buffer!");

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags             = 0; // Create unsignaled
        VkFence tempFence;
        vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &tempFence);
        VkSubmitInfo submitInfo = {};
        submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vkQueueSubmit(device.getQueue(), 0, nullptr, tempFence);

        // TODO: Waits for the GPU Fully to fininish. The fence logic needs to get more complex for efficiency
        vkWaitForFences(device.getDevice(), 1, &tempFence, VK_TRUE, UINT64_MAX);

        size_t sizeInBytes   = elementCount * elementSize;
        size_t offsetInBytes = elementOffset * elementSize;

        std::memcpy(outData, static_cast<uint8_t*>(mappedMemory) + offsetInBytes, sizeInBytes);

        vkDestroyFence(device.getDevice(), tempFence, nullptr);
    }

    VkBuffer& VulcanBuffer::__getBuffer()
    {
        return buffer;
    }
}