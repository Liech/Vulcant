#include "VulcanUi.h"

#include "Library/Vulcant/Wrapper/Graphic/VulcanGraphicCommand.h"
#include "Library/Vulcant/Wrapper/VulcanDevice.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanInstance.h"
#include "Library/Vulcant/Wrapper/VulcanPool.h"
#include "Library/Vulcant/Wrapper/Window.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <stdexcept>

namespace Vulcant::Wrapper
{
    VulcanUi::VulcanUi(VulcanDevice& deviceInput, VulcanPool& poolInput, Window& windowInput)
      : device(deviceInput)
      , pool(poolInput)
      , window(windowInput)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window.getGlfwWindow(), true);

        initDescriptorPool();
        initRenderPass(VK_FORMAT_R8G8B8A8_UNORM);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance                  = device.getInstance().getInstance();
        init_info.PhysicalDevice            = device.getPhysicalDevice();
        init_info.Device                    = device.getDevice();
        init_info.QueueFamily               = device.getQueueFamilyIndex();
        init_info.Queue                     = device.getQueue();
        init_info.DescriptorPool            = descriptorPool;
        init_info.PipelineInfoMain.RenderPass  = renderPass;
        init_info.PipelineInfoMain.Subpass     = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.MinImageCount             = 2;
        init_info.ImageCount                = 2;

        if (!ImGui_ImplVulkan_Init(&init_info))
        {
            throw std::runtime_error("Failed to initialize ImGui Vulkan backend!");
        }
    }

    VulcanUi::~VulcanUi()
    {
        vkDeviceWaitIdle(device.getDevice());

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        for (auto& [view, fb] : framebufferCache)
        {
            if (fb != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device.getDevice(), fb, nullptr);
            }
        }
        framebufferCache.clear();

        if (renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device.getDevice(), renderPass, nullptr);
            renderPass = VK_NULL_HANDLE;
        }

        if (descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device.getDevice(), descriptorPool, nullptr);
            descriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulcanUi::initDescriptorPool()
    {
        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER,                100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       100 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets                    = 1000;
        pool_info.poolSizeCount              = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_info.pPoolSizes                 = pool_sizes;

        if (vkCreateDescriptorPool(device.getDevice(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGui DescriptorPool!");
        }
    }

    void VulcanUi::initRenderPass(VkFormat format)
    {
        renderPassFormat = format;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format                  = format;
        colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment            = 0;
        colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask       = 0;
        dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount        = 1;
        renderPassInfo.pAttachments           = &colorAttachment;
        renderPassInfo.subpassCount           = 1;
        renderPassInfo.pSubpasses             = &subpass;
        renderPassInfo.dependencyCount        = 1;
        renderPassInfo.pDependencies          = &dependency;

        if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGui RenderPass!");
        }
    }

    VkFramebuffer VulcanUi::getOrCreateFramebuffer(VulcanImage& image)
    {
        VkImageView view = image.getView();
        auto        it   = framebufferCache.find(view);
        if (it != framebufferCache.end())
        {
            return it->second;
        }

        VkImageView attachments[] = { view };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass              = renderPass;
        framebufferInfo.attachmentCount         = 1;
        framebufferInfo.pAttachments            = attachments;
        framebufferInfo.width                   = image.getWidth();
        framebufferInfo.height                  = image.getHeight();
        framebufferInfo.layers                  = 1;

        VkFramebuffer fb = VK_NULL_HANDLE;
        if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &fb) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGui Framebuffer!");
        }

        framebufferCache[view] = fb;
        return fb;
    }

    void VulcanUi::newFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void VulcanUi::render(VulcanGraphicCommand& cmd, VulcanImage& targetImage)
    {
        cmd.addBarrier(targetImage, VulcantResourceLayout::ColorAttachment);

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        VkFramebuffer fb = getOrCreateFramebuffer(targetImage);

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass            = renderPass;
        renderPassInfo.framebuffer           = fb;
        renderPassInfo.renderArea.offset     = { 0, 0 };
        renderPassInfo.renderArea.extent     = { targetImage.getWidth(), targetImage.getHeight() };

        VkClearValue clearColor        = { { { 0.1f, 0.1f, 0.15f, 1.0f } } };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues    = &clearColor;

        VkCommandBuffer cmdBuffer = cmd.__getCommandBuffer();
        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(draw_data, cmdBuffer);
        vkCmdEndRenderPass(cmdBuffer);
    }
}
