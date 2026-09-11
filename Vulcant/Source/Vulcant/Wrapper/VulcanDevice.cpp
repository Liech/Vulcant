#include "VulcanDevice.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

#include "VulcanInstance.h"

namespace Vulcant::Wrapper
{
    VulcanDevice::VulcanDevice(VulcanInstance& inputInstance)
      : instance(inputInstance)
    {
        findPhysicalDevice();
        createDevice();
    }

    VulcanDevice::~VulcanDevice()
    {
        vkDestroyDevice(device, NULL);
    }

    VkDevice VulcanDevice::getDevice()
    {
        return device;
    }

    VkPhysicalDevice VulcanDevice::getPhysicalDevice()
    {
        return physicalDevice;
    }

    VulcanInstance& VulcanDevice::getInstance()
    {
        return instance;
    }

    VkQueue VulcanDevice::getQueue()
    {
        return queue;
    }

    uint32_t VulcanDevice::getQueueFamilyIndex()
    {
        return queueFamilyIndex;
    }

    uint32_t VulcanDevice::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;

        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            if ((memoryTypeBits & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
                return i;
        }
        return -1;
    }

    uint32_t VulcanDevice::getComputeQueueFamilyIndex()
    {
        uint32_t queueFamilyCount;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (; i < queueFamilies.size(); ++i)
        {
            VkQueueFamilyProperties props = queueFamilies[i];

            if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                break;
            }
        }

        if (i == queueFamilies.size())
        {
            throw std::runtime_error("could not find a queue family that supports operations");
        }

        return i;
    }

    bool VulcanDevice::isExtensionSupported(const std::string& extensionName) const
    {
        return supportedExtensions.find(extensionName) != supportedExtensions.end();
    }

    void VulcanDevice::checkExtension(const std::string& extensionName) const
    {
        if (!isExtensionSupported(extensionName))
        {
            throw std::runtime_error("Device '" + deviceName + "' is missing required Vulkan extension: " + extensionName);
        }
    }

    void VulcanDevice::checkExtensions(const std::vector<const char*>& extensions) const
    {
        for (const char* ext : extensions)
        {
            checkExtension(ext);
        }
    }

    bool VulcanDevice::isTimelineSemaphoreSupported() const
    {
        if (apiVersion >= VK_API_VERSION_1_2)
        {
            VkPhysicalDeviceVulkan12Features v12{};
            v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            VkPhysicalDeviceFeatures2 f2{};
            f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext = &v12;
            vkGetPhysicalDeviceFeatures2(physicalDevice, &f2);
            if (v12.timelineSemaphore)
                return true;
        }

        if (isExtensionSupported(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME))
        {
            VkPhysicalDeviceTimelineSemaphoreFeaturesKHR sem{};
            sem.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
            VkPhysicalDeviceFeatures2 f2{};
            f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext = &sem;
            vkGetPhysicalDeviceFeatures2(physicalDevice, &f2);
            return sem.timelineSemaphore == VK_TRUE;
        }

        return false;
    }

    void VulcanDevice::createDevice()
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueFamilyIndex                        = getComputeQueueFamilyIndex();
        queueCreateInfo.queueFamilyIndex        = queueFamilyIndex;
        queueCreateInfo.queueCount              = 1;
        float queuePriorities                   = 1.0f;
        queueCreateInfo.pQueuePriorities        = &queuePriorities;

        // 1. Check required extensions
        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        checkExtensions(deviceExtensions);

        // 2. Validate Timeline Semaphore capability
        if (!isTimelineSemaphoreSupported())
        {
            throw std::runtime_error("Device '" + deviceName + "' does not support required feature: 'timelineSemaphore'");
        }

        // 3. Configure Vulkan 1.2 feature chain
        VkPhysicalDeviceVulkan12Features vulkan12Features{};
        vulkan12Features.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12Features.timelineSemaphore = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &vulkan12Features;

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.enabledLayerCount    = static_cast<uint32_t>(instance.getEnabledLayers().size());
        deviceCreateInfo.ppEnabledLayerNames  = instance.getEnabledLayers().data();
        deviceCreateInfo.pQueueCreateInfos    = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;

        deviceCreateInfo.pEnabledFeatures        = nullptr;
        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.pNext                   = &deviceFeatures2;

        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device));

        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    }

    void VulcanDevice::findPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, NULL);
        if (deviceCount == 0)
        {
            throw std::runtime_error("Could not find a device with Vulkan support.");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, devices.data());

        VkPhysicalDevice bestDevice = devices[0];
        int              bestScore  = -1;

        for (VkPhysicalDevice d : devices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(d, &props);

            int score = 0;
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                score += 1000;
            else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                score += 100;

            if (score > bestScore)
            {
                bestScore  = score;
                bestDevice = d;
            }
        }

        physicalDevice = bestDevice;

        VkPhysicalDeviceProperties selectedProps;
        vkGetPhysicalDeviceProperties(physicalDevice, &selectedProps);
        deviceName = selectedProps.deviceName;
        apiVersion = selectedProps.apiVersion;

        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, availableExtensions.data());

        supportedExtensions.clear();
        for (const auto& ext : availableExtensions)
        {
            supportedExtensions.insert(ext.extensionName);
        }

        //std::cout << "[Vulcant] Selected GPU: " << deviceName
        //          << " (Vulkan API: " << VK_API_VERSION_MAJOR(apiVersion) << "."
        //          << VK_API_VERSION_MINOR(apiVersion) << "."
        //          << VK_API_VERSION_PATCH(apiVersion) << ")" << std::endl;
    }
}