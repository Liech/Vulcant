#include "VulcanComputeCommand.h"

#include "Helper/VulcanBarrier.h"
#include "Helper/VulcanCommand.h"
#include "Helper/VulcanTransfer.h"
#include "VulcanBuffer.h"
#include "VulcanDevice.h"
#include "VulcanImage.h"
#include "VulcanInstance.h"
#include "VulcanPool.h"
#include "VulcanSet.h"
#include "VulcanShader.h"

namespace Vulcant::Wrapper
{
    VulcanComputeCommand::VulcanComputeCommand(VulcanPool& poolInput, VulcanDevice& deviceInput)
      : device(deviceInput)
      , pool(poolInput)
    {
        cmd           = std::make_unique<VulcanCommand>(poolInput, device);
        barrier       = std::make_unique<VulcanBarrier>(*cmd);
        transfer      = std::make_unique<VulcanTransfer>(device,*barrier,*cmd);
        currentStatus = status::ready;
    }

    VulcanComputeCommand ::~VulcanComputeCommand()
    {
        if (currentStatus == status::ongpu)
            wait();
    }

    void VulcanComputeCommand::runSync()
    {
        runAsync();
        wait();
    }

    void VulcanComputeCommand::runAsync()
    {
        if (currentStatus == status::ongpu)
            wait();
        assert(currentStatus == status::ready);

        VkSubmitInfo submitInfo       = {};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext              = &barrier->getTimeline();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &cmd->getCommandBuffer();
        submitInfo.waitSemaphoreCount = barrier->getSemaphores().size();
        submitInfo.pWaitSemaphores    = barrier->getSemaphores().data();
        std::vector<VkPipelineStageFlags> waitStages(barrier->getSemaphores().size(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        submitInfo.pWaitDstStageMask    = waitStages.data();
        submitInfo.signalSemaphoreCount = barrier->getSemaphores().size();
        submitInfo.pSignalSemaphores    = barrier->getSemaphores().data();

        VK_CHECK_RESULT(vkQueueSubmit(device.getQueue(), 1, &submitInfo, cmd->getFence()));

        currentStatus = status::ongpu;
    }

    void VulcanComputeCommand::wait()
    {
        assert(currentStatus == status::ongpu);
        cmd->wait();
        transfer->process();

        currentStatus = status::ready;
    }

    void VulcanComputeCommand::startRecord()
    {
        assert(currentStatus == status::ready);
        barrier = std::make_unique<VulcanBarrier>(*cmd);
        cmd->startRecord();
        currentStatus = status::started;
    }

    void VulcanComputeCommand::endRecord()
    {
        assert(currentStatus == status::filled || currentStatus == status::started);
        cmd->endRecord();
        currentStatus = status::ready;
    }

    void VulcanComputeCommand::add(const glm::ivec3& groupCount, VulcanSet& set, VulcanShader& pipeline)
    {
        assert(currentStatus == status::started || currentStatus == status::filled);
        const auto& allSets = set.getSets();
        vkCmdBindPipeline(cmd->getCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getComputePipeline());
        vkCmdBindDescriptorSets(cmd->getCommandBuffer(),
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipeline.getPipelineLayout(),
                                0,                        // firstSet: Start binding from set 0
                                (uint32_t)allSets.size(), // descriptorSetCount: Bind ALL sets in the vector
                                allSets.data(),           // pDescriptorSets: Pointer to the array of sets
                                0,                        // dynamicOffsetCount: 0 for non-dynamic buffers
                                NULL                      // pDynamicOffsets: NULL for non-dynamic buffers
        );
        vkCmdDispatch(cmd->getCommandBuffer(), groupCount.x, groupCount.y, groupCount.z);
        currentStatus = status::filled;
    }

    void VulcanComputeCommand::executeSingleTime(VulcanPool& pool, VulcanDevice& device, std::function<void(VkCommandBuffer)> action)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool        = pool.getCommands();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        action(commandBuffer);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;

        vkQueueSubmit(device.getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.getQueue()); // Warten, bis die GPU fertig ist

        vkFreeCommandBuffers(device.getDevice(), pool.getCommands(), 1, &commandBuffer);
    }

    void VulcanComputeCommand::addBarrier(VulcanBuffer& buffer)
    {
        barrier->addBarrier(buffer);
    }

    void VulcanComputeCommand::addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest)
    {
        barrier->addBarrier(inputImg, dest);
    }

    void VulcanComputeCommand::addCopyBuffer(VulcanBuffer& source, VulcanBuffer& dest, size_t elementCount, size_t sourceOffset, size_t destOffset)
    {
        assert(currentStatus == status::started || currentStatus == status::filled);
        assert(source.getElementSize() == dest.getElementSize());

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset    = sourceOffset * source.getElementSize();
        copyRegion.dstOffset    = destOffset * dest.getElementSize();
        copyRegion.size         = source.getElementSize() * elementCount;

        vkCmdCopyBuffer(cmd->getCommandBuffer(), source.__getBuffer(), dest.__getBuffer(), 1, &copyRegion);

        VkBufferMemoryBarrier barrier = {};
        barrier.sType                 = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask         = VK_ACCESS_SHADER_READ_BIT;
        barrier.buffer                = dest.__getBuffer();
        barrier.size                  = dest.getElementSize() * dest.getNumberOfElements();

        vkCmdPipelineBarrier(cmd->getCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
        currentStatus = status::filled;
    }

    VulcantResourceLayout VulcanComputeCommand::getCurrentLayout(VulcanImage& input) const
    {
        return barrier->getCurrentLayout(input);
    }

    VkCommandBuffer& VulcanComputeCommand::__getCommandBuffer()
    {
        return cmd->getCommandBuffer();
    }
    void VulcanComputeCommand::uploadImageToGPU(VulcanImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset)
    {
        transfer->uploadImageToGPU(destImage, cpuData, extent, offset);
    }

    void VulcanComputeCommand::downloadImageFromGPU(VulcanImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)
    {
        transfer->downloadImageFromGPU(srcImage, outData, extent, offset);
    }
}