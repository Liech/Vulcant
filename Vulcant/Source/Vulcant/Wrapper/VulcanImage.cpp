#include "VulcanImage.h"
#include "VulcanComputeCommand.h"
#include "VulcanDevice.h"
#include "VulcanInstance.h"
#include "VulcanPool.h"
#include "VulcanResource.h"
#include "lodepng.h"
#include <algorithm>
#include <stdexcept>

namespace Vulcant::Wrapper
{
    VulcanImage::VulcanImage(uint32_t widthInput, uint32_t heightInput, uint32_t depthInput, VkFormat formatInput, VulcanPool& poolInput, VulcanDevice& deviceInput)
      : pool(poolInput)
      , device(deviceInput)
      , width(widthInput)
      , height(heightInput)
      , depth(depthInput)
      , format(formatInput)
    {
        createImage();
        createImageView();
        createSemaphore();

        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    }

    VulcanImage::~VulcanImage()
    {
        if (sampler != VK_NULL_HANDLE)
            vkDestroySampler(device.getDevice(), sampler, nullptr);
        vkDestroyImageView(device.getDevice(), imageView, nullptr);
        vkFreeMemory(device.getDevice(), imageMemory, nullptr);
        vkDestroyImage(device.getDevice(), image, nullptr);
        vkDestroySemaphore(device.getDevice(), isReady, nullptr);
    }

    uint32_t VulcanImage::getWidth() const
    {
        return width;
    }

    uint32_t VulcanImage::getHeight() const
    {
        return height;
    }

    void VulcanImage::setUsage(const Vulcant::VulcantImageUsage& inputUsage)
    {
        usage = inputUsage;
        if (sampler == VK_NULL_HANDLE && inputUsage == Vulcant::VulcantImageUsage::SampleOnly)
            createSampler();
    }

    VkDescriptorImageInfo VulcanImage::getInfo() const
    {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView             = imageView;

        if (usage == Vulcant::VulcantImageUsage::SampleOnly)
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler     = sampler;
        }
        else
        {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageInfo.sampler     = VK_NULL_HANDLE;
        }
        return imageInfo;
    }

    void VulcanImage::createImage()
    {

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType         = (depth > 1) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
        imageInfo.extent.width      = width;
        imageInfo.extent.height     = height;
        imageInfo.extent.depth      = depth;
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = 1;
        imageInfo.format            = format;
        imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;

        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (isDepth())
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK_RESULT(vkCreateImage(device.getDevice(), &imageInfo, nullptr, &image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.getDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize       = memRequirements.size;
        allocInfo.memoryTypeIndex      = device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VK_CHECK_RESULT(vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &imageMemory));
        vkBindImageMemory(device.getDevice(), image, imageMemory, 0);
    }

    void VulcanImage::createSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR; // Oder VK_FILTER_NEAREST
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_FALSE; // Für den Anfang auslassen
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void VulcanImage::createImageView()
    {
        VkImageViewCreateInfo viewInfo           = {};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = image;
        viewInfo.viewType                        = (depth > 1) ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = format;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;
        viewInfo.subresourceRange.aspectMask     = isDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VK_CHECK_RESULT(vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &imageView));
    }

    void VulcanImage::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VulcanComputeCommand::executeSingleTime(pool,
                                                device,
                                                [&](VkCommandBuffer cmd)
                                                {
                                                    VkImageMemoryBarrier barrier{};
                                                    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                                                    barrier.oldLayout                       = oldLayout;
                                                    barrier.newLayout                       = newLayout;
                                                    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                                                    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
                                                    barrier.image                           = image;
                                                    barrier.subresourceRange.baseMipLevel   = 0;
                                                    barrier.subresourceRange.levelCount     = 1;
                                                    barrier.subresourceRange.baseArrayLayer = 0;
                                                    barrier.subresourceRange.layerCount     = 1;
                                                    barrier.subresourceRange.aspectMask     = isDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

                                                    // Zugriffsmasken definieren, damit die Pipeline weiß, worauf sie warten muss
                                                    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
                                                    {
                                                        barrier.srcAccessMask = 0;
                                                        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                                                    }
                                                    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                                                    {
                                                        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                                                        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                                                    }

                                                    vkCmdPipelineBarrier(cmd,
                                                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // Worauf warten?
                                                                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Wer wartet?
                                                                         0,
                                                                         0,
                                                                         nullptr,
                                                                         0,
                                                                         nullptr,
                                                                         1,
                                                                         &barrier);
                                                });
    }

    void VulcanImage::saveRenderedImage(const std::string& path)
    {
        assert(depth == 1);
        VkFormat dstFormat = VK_FORMAT_R8G8B8A8_UNORM;

        // 1. Create Intermediate Image (Device Local)
        VkImage           dstImage;
        VkDeviceMemory    dstImageMemory;
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType         = VK_IMAGE_TYPE_2D;
        imageInfo.format            = dstFormat;
        imageInfo.extent            = { width, height, 1 };
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = 1;
        imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage             = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK_RESULT(vkCreateImage(device.getDevice(), &imageInfo, nullptr, &dstImage));

        VkMemoryRequirements imgMemReqs;
        vkGetImageMemoryRequirements(device.getDevice(), dstImage, &imgMemReqs);

        VkMemoryAllocateInfo imgAlloc = {};
        imgAlloc.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        imgAlloc.allocationSize       = imgMemReqs.size;
        // CHANGE: Images used for blitting usually need to be DEVICE_LOCAL_BIT
        imgAlloc.memoryTypeIndex = device.findMemoryType(imgMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device.getDevice(), &imgAlloc, nullptr, &dstImageMemory));
        vkBindImageMemory(device.getDevice(), dstImage, dstImageMemory, 0);

        // 2. Create Staging Buffer (Host Visible)
        VkDeviceSize       imageSize = width * height * 4;
        VkBuffer           stagingBuffer;
        VkDeviceMemory     stagingBufferMemory;
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size               = imageSize;
        bufferInfo.usage              = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK_RESULT(vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &stagingBuffer));

        VkMemoryRequirements bufMemReqs;
        vkGetBufferMemoryRequirements(device.getDevice(), stagingBuffer, &bufMemReqs);

        VkMemoryAllocateInfo bufAlloc = {};
        bufAlloc.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        bufAlloc.allocationSize       = bufMemReqs.size;
        // Staging buffers MUST be HOST_VISIBLE to be mapped
        bufAlloc.memoryTypeIndex = device.findMemoryType(bufMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device.getDevice(), &bufAlloc, nullptr, &stagingBufferMemory));
        vkBindBufferMemory(device.getDevice(), stagingBuffer, stagingBufferMemory, 0);

        // 3. Execute Commands with Transitions
        VulcanComputeCommand::executeSingleTime(
          pool,
          device,
          [&](VkCommandBuffer cmd)
          {
              // Helper to transition layouts
              auto transition = [&](VkImage img, VkImageLayout oldL, VkImageLayout newL, VkAccessFlags srcAcc, VkAccessFlags dstAcc, VkPipelineStageFlags srcStg, VkPipelineStageFlags dstStg)
              {
                  VkImageMemoryBarrier barrier = {};
                  barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                  barrier.oldLayout            = oldL;
                  barrier.newLayout            = newL;
                  barrier.srcAccessMask        = srcAcc;
                  barrier.dstAccessMask        = dstAcc;
                  barrier.image                = img;
                  barrier.subresourceRange     = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                  vkCmdPipelineBarrier(cmd, srcStg, dstStg, 0, 0, nullptr, 0, nullptr, 1, &barrier);
              };

              // Transition: Src Image -> SRC_OPTIMAL, Dst Image -> DST_OPTIMAL
              transition(image,
                         VK_IMAGE_LAYOUT_GENERAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_ACCESS_SHADER_WRITE_BIT,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT);
              transition(dstImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

              VkImageBlit blit{};
              blit.srcOffsets[1]  = { static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };
              blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
              blit.dstOffsets[1]  = { static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };
              blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

              vkCmdBlitImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

              // Transition: Dst Image -> SRC_OPTIMAL for buffer copy
              transition(dstImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT);

              VkBufferImageCopy region = {};
              region.imageSubresource  = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
              region.imageExtent       = { width, height, 1 };
              vkCmdCopyImageToBuffer(cmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

              // Cleanup: Transition original image back to GENERAL
              transition(image,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_IMAGE_LAYOUT_GENERAL,
                         VK_ACCESS_TRANSFER_READ_BIT,
                         VK_ACCESS_SHADER_WRITE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
          });

        // 4. Map, Save, and Cleanup
        void* data;
        vkMapMemory(device.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        lodepng::encode(path, static_cast<unsigned char*>(data), width, height);
        vkUnmapMemory(device.getDevice(), stagingBufferMemory);

        vkDestroyBuffer(device.getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(device.getDevice(), stagingBufferMemory, nullptr);
        vkDestroyImage(device.getDevice(), dstImage, nullptr);
        vkFreeMemory(device.getDevice(), dstImageMemory, nullptr);
    }

    VulcanResource VulcanImage::asResource() const
    {
        VkDescriptorType t = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        if (usage == Vulcant::VulcantImageUsage::SampleOnly)
        {
            t = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return { t, {}, getInfo() };
    }

    VkImage& VulcanImage::__getImage()
    {
        return image;
    }

    bool VulcanImage::isDepth() const
    {
        return VK_FORMAT_D32_SFLOAT == format;
    }

    VkSemaphore VulcanImage::__getSemaphore()
    {
        return isReady;
    }

    size_t& VulcanImage::__getTimeline()
    {
        return timeline;
    }

    VkFormat VulcanImage::getFormat() const
    {
        return format;
    }

    void VulcanImage::createSemaphore()
    {
        // 1. Timeline-Spezifikation definieren
        VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo{};
        semaphoreTypeCreateInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphoreTypeCreateInfo.pNext         = nullptr;
        semaphoreTypeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphoreTypeCreateInfo.initialValue  = timeline;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &semaphoreTypeCreateInfo;
        semaphoreInfo.flags = 0;

        if (vkCreateSemaphore(device.getDevice(), &semaphoreInfo, nullptr, &isReady) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create timeline semaphore");
        }
    }

    VkImageView VulcanImage::getView() const
    {
        return imageView;
    }
}