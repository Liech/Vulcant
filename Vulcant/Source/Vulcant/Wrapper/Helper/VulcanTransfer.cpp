#include "VulcanTransfer.h"

#include "Library/Vulcant/Wrapper/Helper/VulcanCommand.h"
#include "Library/Vulcant/Wrapper/Helper/VulcanBarrier.h"
#include "Library/Vulcant/Wrapper/VulcanDevice.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanInstance.h"
#include <cassert>

namespace Vulcant::Wrapper
{
    VulcanTransfer::VulcanTransfer(VulcanDevice& deviceInput, VulcanBarrier& barrierInput, VulcanCommand& cmdInput)
      : device(deviceInput)
      , barrier(barrierInput)
      , cmd(cmdInput)
    {
    }

    VulcanTransfer::~VulcanTransfer() {}

    void VulcanTransfer::uploadImageToGPU(VulcanImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset)
    {
        assert(barrier.getCurrentLayout(destImage) == VulcantResourceLayout::TransferDst);

        VkExtent3D   vkExtent   = { extent.x, extent.y, extent.z };
        VkOffset3D   vkOffset   = { static_cast<int32_t>(offset.x), static_cast<int32_t>(offset.y), static_cast<int32_t>(offset.z) };
        VkDeviceSize bufferSize = extent.x * extent.y * extent.z * 4;

        // 1. Create Staging Buffer
        VkBuffer       stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size  = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VK_CHECK_RESULT(vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &stagingBuffer));

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device.getDevice(), stagingBuffer, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memReqs.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &stagingBufferMemory));
        vkBindBufferMemory(device.getDevice(), stagingBuffer, stagingBufferMemory, 0);

        // 2. Map & Copy CPU Data
        void* mappedData;
        vkMapMemory(device.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedData);
        std::memcpy(mappedData, cpuData, static_cast<size_t>(bufferSize));
        vkUnmapMemory(device.getDevice(), stagingBufferMemory);

        VkBufferImageCopy region{};
        region.imageOffset                 = vkOffset;
        region.imageExtent                 = vkExtent;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;

        vkCmdCopyBufferToImage(cmd.getCommandBuffer(), stagingBuffer, destImage.__getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // defer cleanu
        activeStagingResources.push_back({ stagingBuffer, stagingBufferMemory });
    }

    void VulcanTransfer::downloadImageFromGPU(VulcanImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)
    {
        assert(barrier.getCurrentLayout(srcImage) == VulcantResourceLayout::TransferSrc);

        VkExtent3D   vkExtent   = { extent.x, extent.y, extent.z };
        VkOffset3D   vkOffset   = { static_cast<int32_t>(offset.x), static_cast<int32_t>(offset.y), static_cast<int32_t>(offset.z) };
        VkDeviceSize bufferSize = extent.x * extent.y * extent.z * 4; // Assumes 4 bytes per pixel

        // 1. Create a Staging Buffer to receive the GPU data
        VkBuffer       stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size  = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT; // Destination of a transfer
        VK_CHECK_RESULT(vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &stagingBuffer));

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device.getDevice(), stagingBuffer, &memReqs);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memReqs.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &stagingBufferMemory));
        vkBindBufferMemory(device.getDevice(), stagingBuffer, stagingBufferMemory, 0);

        // 3. Record Image to Buffer Copy
        VkBufferImageCopy region{};
        region.bufferOffset                = 0;
        region.bufferRowLength             = 0;
        region.bufferImageHeight           = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageOffset                 = vkOffset;
        region.imageExtent                 = vkExtent;

        vkCmdCopyImageToBuffer(cmd.getCommandBuffer(), srcImage.__getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

        // Defer processing until the fence is hit
        activeDownloadResources.push_back({ stagingBuffer, stagingBufferMemory, outData, static_cast<size_t>(bufferSize) });
    }

    void VulcanTransfer::process()
    {
        for (auto& resource : activeStagingResources)
        {
            vkDestroyBuffer(device.getDevice(), resource.buffer, nullptr);
            vkFreeMemory(device.getDevice(), resource.memory, nullptr);
        }
        activeStagingResources.clear();

        for (auto& download : activeDownloadResources)
        {
            void* mappedData;
            vkMapMemory(device.getDevice(), download.memory, 0, download.dataSize, 0, &mappedData);
            std::memcpy(download.outCpuPtr, mappedData, download.dataSize);
            vkUnmapMemory(device.getDevice(), download.memory);
            vkDestroyBuffer(device.getDevice(), download.buffer, nullptr);
            vkFreeMemory(device.getDevice(), download.memory, nullptr);
        }
        activeDownloadResources.clear();
    }

}