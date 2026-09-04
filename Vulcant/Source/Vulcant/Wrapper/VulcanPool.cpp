#include "VulcanPool.h"

#include "VulcanDevice.h"
#include "VulcanInstance.h"
#include "VulcanSet.h"

namespace Vulcant::Wrapper
{
    VulcanPool::VulcanPool(VulcanDevice& deviceInput, uint32_t spaceInput)
      : device(deviceInput)
      , space(spaceInput)
    {
        createDescriptorPool();
        createComputeCommandPool();
    }

    VulcanPool::~VulcanPool()
    {
        vkDestroyDescriptorPool(device.getDevice(), descriptorPool, NULL);
        vkDestroyCommandPool(device.getDevice(), commandPool, NULL);
    }

    void VulcanPool::reset()
    {
        for (auto& x : tracked)
            x->invalidate();
        tracked.clear();
        vkResetDescriptorPool(device.getDevice(), descriptorPool, 0);
        vkResetCommandPool(device.getDevice(), commandPool, 0);
    }

    void VulcanPool::track(VulcanSet& toTrack)
    {
        tracked.insert(&toTrack);
    }

    void VulcanPool::untrack(VulcanSet& toUntrack)
    {
        tracked.erase(&toUntrack);
    }
    VkDescriptorPool VulcanPool::getDescriptor()
    {
        return descriptorPool;
    }

    VkCommandPool VulcanPool::getCommands()
    {
        return commandPool;
    }

    VulcanDevice& VulcanPool::getDevice()
    {
        return device;
    }

    void VulcanPool::createComputeCommandPool()
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags                   = 0;
        // the queue family of this command pool. All command buffers allocated from this command pool,
        // must be submitted to queues of this family ONLY.
        commandPoolCreateInfo.queueFamilyIndex = device.getQueueFamilyIndex();
        VK_CHECK_RESULT(vkCreateCommandPool(device.getDevice(), &commandPoolCreateInfo, NULL, &commandPool));
    }

    void VulcanPool::createDescriptorPool()
    {
        // Define sizes for all types your shaders might use
        std::vector<VkDescriptorPoolSize> poolSizes = {
            // This is what was missing!
            {          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, space },
            // This is what you had before
            {         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, space },
            // Good to have these just in case
            {         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, space },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, space }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount              = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes                 = poolSizes.data();

        // Ensure maxSets is at least as large as the number of sets you allocate
        poolInfo.maxSets = space;

        VK_CHECK_RESULT(vkCreateDescriptorPool(device.getDevice(), &poolInfo, NULL, &descriptorPool));
    }
}