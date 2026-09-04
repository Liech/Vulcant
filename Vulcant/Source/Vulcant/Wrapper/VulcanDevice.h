#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanInstance;

    class VulcanDevice
    {
      public:
        VulcanDevice(VulcanInstance& instance);
        virtual ~VulcanDevice();

        VkDevice         getDevice();
        VkPhysicalDevice getPhysicalDevice();
        VkQueue          getQueue();
        uint32_t         getQueueFamilyIndex();
        VulcanInstance&  getInstance();

        bool isExtensionSupported(const std::string& extensionName) const;
        void checkExtension(const std::string& extensionName) const;
        void checkExtensions(const std::vector<const char*>& extensions) const;

        bool isTimelineSemaphoreSupported() const;

        uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);

      private:
        void     findPhysicalDevice();
        void     createDevice();
        uint32_t getComputeQueueFamilyIndex();

        VulcanInstance& instance;

        VkDevice                        device;
        VkPhysicalDevice                physicalDevice;
        VkQueue                         queue;
        uint32_t                        queueFamilyIndex;
        std::unordered_set<std::string> supportedExtensions;
        std::string                     deviceName;
        uint32_t                        apiVersion = 0;
    };
}