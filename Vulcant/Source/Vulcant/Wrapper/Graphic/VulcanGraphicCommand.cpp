#include "VulcanGraphicCommand.h"
#include "Library/Vulcant/Wrapper/VulcanDevice.h"
#include "Library/Vulcant/Wrapper/VulcanInstance.h"
#include "Library/Vulcant/Wrapper/VulcanPool.h"
#include <array>
#include <cassert>

namespace Vulcant::Wrapper
{
    VulcanGraphicCommand::VulcanGraphicCommand(VulcanPool& poolInput, VulcanDevice& deviceInput)
      : device(deviceInput)
      , pool(poolInput)
    {
        cmd           = std::make_unique<VulcanCommand>(poolInput, device);
        barrier       = std::make_unique<VulcanBarrier>(*cmd);
        currentStatus = status::ready;
    }

    VulcanGraphicCommand::~VulcanGraphicCommand()
    {
        if (currentStatus == status::ongpu)
            wait();
    }

    void VulcanGraphicCommand::startRecord()
    {
        if (currentStatus == status::ongpu)
            wait();

        assert(currentStatus == status::ready);
        barrier = std::make_unique<VulcanBarrier>(*cmd);
        cmd->startRecord();
        currentStatus = status::started;
    }

    void VulcanGraphicCommand::beginRendering(VulcanGraphicPipeline& pipeline)
    {
        assert(currentStatus == status::started);
        assert(currentPipeline == nullptr);
        
        currentPipeline = &pipeline;

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = pipeline.getRenderPass();
        renderPassInfo.framebuffer = pipeline.getFramebuffer();
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { pipeline.getExtent().x, pipeline.getExtent().y };

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color        = { { 0.1f, 0.1f, 0.1f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues    = clearValues.data();

        vkCmdBeginRenderPass(cmd->getCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        currentStatus = status::inRenderPass;
    }

    void VulcanGraphicCommand::setViewportAndScissor(glm::uvec2 extent)
    {
        assert(currentStatus == status::inRenderPass);

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(extent.x);
        viewport.height   = static_cast<float>(extent.y);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd->getCommandBuffer(), 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { extent.x, extent.y };
        vkCmdSetScissor(cmd->getCommandBuffer(), 0, 1, &scissor);
    }

    void VulcanGraphicCommand::draw(uint32_t vertexCount, VulcanSet* set, VulcanBuffer* vertexBuffer)
    {
        assert(currentStatus == status::inRenderPass);

        vkCmdBindPipeline(cmd->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getPipeline());

        if (set)
        {
            const auto& allSets = set->getSets();
            vkCmdBindDescriptorSets(cmd->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->getLayout(), 0, static_cast<uint32_t>(allSets.size()), allSets.data(), 0, nullptr);
        }

        if (vertexBuffer)
        {
            VkBuffer     buffers[] = { vertexBuffer->__getBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(cmd->getCommandBuffer(), 0, 1, buffers, offsets);
        }

        vkCmdDraw(cmd->getCommandBuffer(), vertexCount, 1, 0, 0);
    }

    void VulcanGraphicCommand::endRendering()
    {
        assert(currentStatus == status::inRenderPass);
        vkCmdEndRenderPass(cmd->getCommandBuffer());
        currentPipeline = nullptr;
        currentStatus = status::filled;
    }

    void VulcanGraphicCommand::endRecord()
    {
        assert(currentStatus == status::filled || currentStatus == status::started);
        cmd->endRecord();
        currentStatus = status::ready;
    }

    void VulcanGraphicCommand::addBarrier(VulcanBuffer& buffer)
    {
        barrier->addBarrier(buffer);
    }

    void VulcanGraphicCommand::addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest)
    {
        barrier->addBarrier(inputImg, dest);
    }

    void VulcanGraphicCommand::runAsync()
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
        std::vector<VkPipelineStageFlags> waitStages(barrier->getSemaphores().size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        submitInfo.pWaitDstStageMask    = waitStages.data();
        submitInfo.signalSemaphoreCount = barrier->getSemaphores().size();
        submitInfo.pSignalSemaphores    = barrier->getSemaphores().data();

        VK_CHECK_RESULT(vkQueueSubmit(device.getQueue(), 1, &submitInfo, cmd->getFence()));

        currentStatus = status::ongpu;
    }

    void VulcanGraphicCommand::runSync()
    {
        runAsync();
        wait();
    }

    void VulcanGraphicCommand::wait()
    {
        assert(currentStatus == status::ongpu);
        cmd->wait();
        currentStatus = status::ready;
    }
}