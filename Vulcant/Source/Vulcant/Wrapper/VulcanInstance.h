#pragma once

#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>
#include <string>

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                                                                                                                                                                             \
    {                                                                                                                                                                                                  \
        VkResult res = (f);                                                                                                                                                                            \
        if (res != VK_SUCCESS)                                                                                                                                                                         \
        {                                                                                                                                                                                              \
            printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, __LINE__);                                                                                                              \
            assert(res == VK_SUCCESS);                                                                                                                                                                 \
        }                                                                                                                                                                                              \
    }

namespace Vulcant::Wrapper
{
    class VulcanInstance
    {
      public:
        VulcanInstance(const std::vector<std::string>& extensions = {},bool debug = true);
        virtual ~VulcanInstance();

        bool                      isDebug() const;
        VkInstance                getInstance();
        std::vector<const char*>& getEnabledLayers();

      private:
        void initValidationLayer();
        void initInstance();
        void initDebugMessages();

        VkInstance               instance;
        bool                     debug = false;
        std::vector<const char*> enabledExtensions;
        std::vector<std::string> additionalExtensions;
        std::vector<const char*> enabledLayers;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void*                                       pUserData)
        {
            fprintf(stderr, "Validation Layer: %s\n", pCallbackData->pMessage);
            return VK_FALSE;
        }
    };
}