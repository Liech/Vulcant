#pragma once
#include "Library/Vulcant/Interface/VulcantImageUsage.h"
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanPool;
    class VulcanRessource;
    struct VulcanResource;

    class VulcanImage
    {
      public:
        VulcanImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VulcanPool& pool, VulcanDevice& device);
        VulcanImage(VkImage img, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VulcanPool& pool, VulcanDevice& device);
        VulcanImage& operator=(const VulcanImage&)           = delete;
        VulcanImage(VulcanImage&& other) noexcept            = default;
        VulcanImage& operator=(VulcanImage&& other) noexcept = default;
        virtual ~VulcanImage();

        VkImageView           getView() const;
        VkDescriptorImageInfo getInfo() const;
        void                  saveRenderedImage(const std::string& path);

        uint32_t getWidth() const;
        uint32_t getHeight() const;
        void     setUsage(const Vulcant::VulcantImageUsage& usage);
        bool     isDepth() const;

        VulcanResource asResource() const;

        VkSemaphore __getSemaphore();
        size_t&      __getTimeline();
        VkImage&    __getImage();
        VkFormat     getFormat() const;

      private:
        void createImage();
        void createSampler();
        void createImageView();
        void createSemaphore();

        // Hilfsfunktion: Layout-Wechsel (z.B. für Compute oder Transfer)
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        VulcanDevice& device;
        VulcanPool&   pool;
        uint32_t      width, height, depth;
        VkFormat      format;

        VkImage        image;
        VkDeviceMemory imageMemory;
        VkImageView    imageView;
        VkSampler      sampler = VK_NULL_HANDLE;
        VkSemaphore    isReady;
        size_t         timeline = 0;

        Vulcant::VulcantImageUsage usage = Vulcant::VulcantImageUsage::WriteAndRead;
    };
}