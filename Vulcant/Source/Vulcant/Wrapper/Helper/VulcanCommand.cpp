#include "VulcanCommand.h"

#include "Library/Vulcant/Wrapper/VulcanDevice.h"
#include "Library/Vulcant/Wrapper/VulcanInstance.h"
#include "Library/Vulcant/Wrapper/VulcanPool.h"

namespace Vulcant::Wrapper
{
    VulcanCommand::VulcanCommand(VulcanPool& poolInput, VulcanDevice& deviceInput)
      : device(deviceInput)
      , pool(poolInput)
    {
        state = status::fresh;
        createFence();
    }

    VulcanCommand::~VulcanCommand()
    {
        assert(state != status::recording);
        destroyFence();
    }

    void VulcanCommand::startRecord()
    {
        assert(state == status::fresh || state == status::ready);

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool                 = pool.getCommands();
        commandBufferAllocateInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount          = 1;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device.getDevice(), &commandBufferAllocateInfo, &commandBuffer));
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        state = status::recording;
    }

    void VulcanCommand::createFence()
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags             = 0;
        VK_CHECK_RESULT(vkCreateFence(device.getDevice(), &fenceCreateInfo, NULL, &fence));
    }

    void VulcanCommand::destroyFence()
    {
        vkDestroyFence(device.getDevice(), fence, NULL);
    }

    void VulcanCommand::wait()
    {
        VK_CHECK_RESULT(vkWaitForFences(device.getDevice(), 1, &fence, VK_TRUE, 100000000000));
        VK_CHECK_RESULT(vkResetFences(device.getDevice(), 1, &fence));
    }

    void VulcanCommand::endRecord()
    {
        assert(state == status::recording);
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
        state = status::ready;
    }

    VkFence& VulcanCommand::getFence()
    {
        return fence;
    }

    VkCommandBuffer& VulcanCommand::getCommandBuffer()
    {
        return commandBuffer;
    }
}