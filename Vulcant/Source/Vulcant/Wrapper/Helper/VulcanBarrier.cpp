#include "VulcanBarrier.h"
#include "Library/Vulcant/Wrapper/VulcanBuffer.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "VulcanCommand.h"

namespace Vulcant::Wrapper
{
    VulcanBarrier::VulcanBarrier(VulcanCommand& cmdInput)
      : cmd(cmdInput)
    {
    }

 VulcanBarrier ::~VulcanBarrier() {}

    VkTimelineSemaphoreSubmitInfo& VulcanBarrier::getTimeline()
    {
        waitValues.clear();
        signalValues.clear();
        semaphores.clear();
        for (auto* resource : tracked)
        {
            auto& t = resource->__getTimeline();
            semaphores.push_back(resource->__getSemaphore());

            waitValues.push_back(t);
            t++;
            signalValues.push_back(t);
        }

        timelineInfo.sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.waitSemaphoreValueCount   = waitValues.size();
        timelineInfo.signalSemaphoreValueCount = signalValues.size();
        timelineInfo.pWaitSemaphoreValues      = !waitValues.empty() ? waitValues.data() : nullptr;
        timelineInfo.pSignalSemaphoreValues    = !signalValues.empty() ? signalValues.data() : nullptr;
        timelineInfo.pNext                     = nullptr;
        created                                = true;

        return timelineInfo;
    }

    const std::vector<VkSemaphore>& VulcanBarrier::getSemaphores()
    {
        return semaphores;
    }

    void VulcanBarrier::addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest)
    {
        VkImage img = inputImg.__getImage();
        tracked.insert(&inputImg);

        VulcantResourceLayout source = VulcantResourceLayout::Undefined;
        if (formerLayouts.contains(img))
        {
            source = formerLayouts[img];
        }
        addBarrier(inputImg, source, dest); 
        formerLayouts[img] = dest;
    }
    VulcantResourceLayout VulcanBarrier::getCurrentLayout(VulcanImage& input)
    {
        VkImage img = input.__getImage();
        if (formerLayouts.contains(img))
        {
            return formerLayouts.at(img);
        }
        return VulcantResourceLayout::Undefined;
    }

    void VulcanBarrier::addBarrier(VulcanBuffer& buffer)
    {
        VkBufferMemoryBarrier barrier{};
        barrier.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.buffer        = buffer.__getBuffer();
        barrier.size          = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(cmd.getCommandBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    }

    void VulcanBarrier::addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& old_layout, const VulcantResourceLayout& new_layout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout           = VulcanBarrier::mapLayout(old_layout);
        barrier.newLayout           = VulcanBarrier::mapLayout(new_layout);
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = inputImg.__getImage();

        barrier.subresourceRange.aspectMask     = inputImg.isDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;


        if (old_layout == VulcantResourceLayout::Undefined && new_layout == VulcantResourceLayout::General)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            srcStage              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        // Neu: Übergang zu Color Attachment (Graphics RenderPass Start)
        else if (new_layout == VulcantResourceLayout::ColorAttachment)
        {
            barrier.srcAccessMask = (old_layout == VulcantResourceLayout::General) ? VK_ACCESS_SHADER_WRITE_BIT : 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            srcStage              = (old_layout == VulcantResourceLayout::General) ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        // Neu: Übergang zu Depth/Stencil Attachment (Depth Pass Start)
        else if (new_layout == VulcantResourceLayout::DepthStencilAttachment)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage              = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        // Raycast (Compute Write) -> Deferred (Fragment Read)
        else if (old_layout == VulcantResourceLayout::General && new_layout == VulcantResourceLayout::ShaderReadOnly)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dstStage              = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        // Render-Target -> Shader Read
        else if (old_layout == VulcantResourceLayout::ColorAttachment && new_layout == VulcantResourceLayout::ShaderReadOnly)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dstStage              = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        // read -> present
        else if (old_layout == VulcantResourceLayout::ShaderReadOnly && new_layout == VulcantResourceLayout::Present)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = 0;
            srcStage              = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dstStage              = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }
        // copy
        else if (new_layout == VulcantResourceLayout::TransferDst)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage              = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else
        {
            // Safety fallback
            barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
            srcStage              = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            dstStage              = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }

        vkCmdPipelineBarrier(cmd.getCommandBuffer(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    VkImageLayout VulcanBarrier::mapLayout(VulcantResourceLayout layout)
    {
        switch (layout)
        {
            case VulcantResourceLayout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case VulcantResourceLayout::General:
                return VK_IMAGE_LAYOUT_GENERAL;
            case VulcantResourceLayout::ColorAttachment:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case VulcantResourceLayout::DepthStencilAttachment:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case VulcantResourceLayout::ShaderReadOnly:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case VulcantResourceLayout::TransferSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case VulcantResourceLayout::TransferDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case VulcantResourceLayout::Present:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            default:
                return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }
}