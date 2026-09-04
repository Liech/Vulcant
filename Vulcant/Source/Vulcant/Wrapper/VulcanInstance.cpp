#include "VulcanInstance.h"

#include <stdexcept>

namespace Vulcant::Wrapper
{
    VulcanInstance::VulcanInstance(const std::vector<std::string>& extensions, bool debugInput)
    {
        additionalExtensions = extensions;
        for (const auto& x : additionalExtensions)
            enabledExtensions.push_back(x.c_str());


        if (debugInput)
            initValidationLayer();
        initInstance();
        if (debugInput)
            initDebugMessages();
    }

    VulcanInstance::~VulcanInstance()
    {
        if (debugMessenger != VK_NULL_HANDLE)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                func(instance, debugMessenger, nullptr);
            }
        }
        vkDestroyInstance(instance, NULL);
    }

    void VulcanInstance::initInstance()
    {
        VkApplicationInfo applicationInfo  = {};
        applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pApplicationName   = "NPH";
        applicationInfo.applicationVersion = 0;
        applicationInfo.pEngineName        = "NPH";
        applicationInfo.engineVersion      = 0;
        applicationInfo.apiVersion         = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags                = 0;
        createInfo.pApplicationInfo     = &applicationInfo;

        createInfo.enabledLayerCount       = enabledLayers.size();
        createInfo.ppEnabledLayerNames     = enabledLayers.data();
        createInfo.enabledExtensionCount   = enabledExtensions.size();
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &instance));
    }

    VkInstance VulcanInstance::getInstance()
    {
        return instance;
    }

    bool VulcanInstance::isDebug() const
    {
        return debug;
    }

    void VulcanInstance::initValidationLayer()
    {
        debug = true;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);

        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        bool foundLayer = false;
        for (const VkLayerProperties& prop : layerProperties)
        {

            if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0)
            {
                foundLayer = true;
                break;
            }
        }

        if (!foundLayer)
        {
            throw std::runtime_error("Layer VK_LAYER_KHRONOS_validation not supported\n");
        }
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

        uint32_t extensionCount;

        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

        bool foundExtension = false;
        for (const VkExtensionProperties& prop : extensionProperties)
        {
            if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, prop.extensionName) == 0)
            {
                foundExtension = true;
                break;
            }
        }

        if (!foundExtension)
        {
            throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
        }
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void*                                       pUserData)
    {
        // Highlight Errors in a different way if you want
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            fprintf(stderr, "VULKAN VALIDATION: %s\n\n", pCallbackData->pMessage);
            fflush(stdout);
        }

        return VK_FALSE; // true -> crash
    }

    void VulcanInstance::initDebugMessages()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // Select what you want to hear about
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData       = nullptr; // Optional

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr)
        {
            VK_CHECK_RESULT(func(instance, &createInfo, NULL, &debugMessenger));
        }
        else
        {
            throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT. Is the extension enabled?");
        }
    }

    std::vector<const char*>& VulcanInstance::getEnabledLayers()
    {
        return enabledLayers;
    }
}