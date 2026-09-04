#include "VulcanGraphicPipeline.h"

#include "Library/Vulcant/Wrapper/Graphic/GraphicPipelineGenerator.h"
#include "Library/Vulcant/Wrapper/VulcanDevice.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanInstance.h"
#include "Library/Vulcant/Wrapper/VulcanShader.h"
#include <array>
#include <stdexcept>

namespace Vulcant::Wrapper
{
    VulcanGraphicPipeline::VulcanGraphicPipeline(VulcanDevice& deviceInput, const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
      : device(deviceInput)
    {
        createRenderPass(color, depth, stencil);
        createFramebuffer(color, depth, stencil);
        createPipeline(shader, color, depth, stencil);
    }

    VulcanGraphicPipeline::~VulcanGraphicPipeline()
    {
        if (pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
        }
        if (framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device.getDevice(), framebuffer, nullptr);
        }
        if (renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device.getDevice(), renderPass, nullptr);
        }
    }

    VkPipeline VulcanGraphicPipeline::getPipeline()
    {
        return pipeline;
    }

    VkPipelineLayout VulcanGraphicPipeline::getLayout()
    {
        return layout;
    }

    VkRenderPass VulcanGraphicPipeline::getRenderPass()
    {
        return renderPass;
    }

    VkFramebuffer VulcanGraphicPipeline::getFramebuffer()
    {
        return framebuffer;
    }

    glm::uvec2 VulcanGraphicPipeline::getExtent() const
    {
        return extent;
    }

    void VulcanGraphicPipeline::createRenderPass(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
    {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference>   colorAttachmentRefs;

        for (size_t i = 0; i < color.size(); i++)
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format         = color[i]->getFormat();
            colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

            attachments.push_back(colorAttachment);

            VkAttachmentReference colorRef{};
            colorRef.attachment = static_cast<uint32_t>(i);
            colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs.push_back(colorRef);
        }

        VkAttachmentReference depthAttachmentRef{};
        if (depth)
        {
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format         = depth->getFormat();
            depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthAttachmentRef.attachment = static_cast<uint32_t>(attachments.size());
            depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments.push_back(depthAttachment);
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = static_cast<uint32_t>(colorAttachmentRefs.size());
        subpass.pColorAttachments       = colorAttachmentRefs.data();
        subpass.pDepthStencilAttachment = depth ? &depthAttachmentRef : nullptr;

        VkSubpassDependency dependency{};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | (depth ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : 0);
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | (depth ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT : 0);
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | (depth ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VK_CHECK_RESULT(vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &renderPass));
    }

    void VulcanGraphicPipeline::createFramebuffer(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
    {
        std::vector<VkImageView> attachments;
        for (auto* c : color)
        {
            attachments.push_back(c->getView());
        }
        if (depth)
        {
            attachments.push_back(depth->getView());
        }

        extent = { color[0]->getWidth(), color[0]->getHeight() };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = extent.x;
        framebufferInfo.height          = extent.y;
        framebufferInfo.layers          = 1;

        VK_CHECK_RESULT(vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &framebuffer));
    }

    void VulcanGraphicPipeline::createPipeline(const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
    {
        Vulcant::Wrapper::Graphic::GraphicPipelineGenerator pipeOptions;
        pipeOptions.setBoiler();
        pipeOptions.setColor(color);
        pipeOptions.setDepthStencil(depth, stencil);
        pipeOptions.setShader(shader);
        pipeOptions.setRenderPass(renderPass, 0);

        VkGraphicsPipelineCreateInfo& pipeInfo = pipeOptions.get();

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipeInfo, nullptr, &pipeline));
        layout = pipeInfo.layout;
    }
}