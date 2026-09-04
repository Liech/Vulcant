#pragma once

#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanPool;

    class VulcanCommand
    {
      public:
        VulcanCommand(VulcanPool& pool, VulcanDevice& deviceInput);
        virtual ~VulcanCommand();

        void wait();
        void startRecord();
        void endRecord();

        VkFence&         getFence();
        VkCommandBuffer& getCommandBuffer();

      private:
        void createFence();
        void destroyFence();

        enum class status
        {
            fresh,
            recording,
            ready
        };
        status state = status::fresh;

        VulcanDevice& device;
        VulcanPool&   pool;

        VkFence         fence;
        VkCommandBuffer commandBuffer;
    };
}