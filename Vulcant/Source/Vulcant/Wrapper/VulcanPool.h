#pragma once

#include <set>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanSet;

    class VulcanPool
    {
      public:
        VulcanPool(VulcanDevice& device, uint32_t space = 10);
        virtual ~VulcanPool();

        void reset();

        VkDescriptorPool getDescriptor();
        VkCommandPool    getCommands();
        VulcanDevice&    getDevice();

        void track(VulcanSet&);
        void untrack(VulcanSet&);

      private:
        void createDescriptorPool();
        void createComputeCommandPool();

        VulcanDevice& device;
        uint32_t      space;

        VkDescriptorPool     descriptorPool;
        VkCommandPool        commandPool;

        std::set<VulcanSet*> tracked;
    };
}