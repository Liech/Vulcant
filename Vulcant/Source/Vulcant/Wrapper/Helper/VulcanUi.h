#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanPool;
    class VulcanImage;
    class Window;
    class VulcanGraphicCommand;

    class VulcanUi
    {
      public:
        VulcanUi(VulcanDevice& device, VulcanPool& pool, Window& window);
        virtual ~VulcanUi();

        void newFrame();
        void render(VulcanGraphicCommand& cmd, VulcanImage& targetImage);

      private:
        void initDescriptorPool();
        void initRenderPass(VkFormat format);
        VkFramebuffer getOrCreateFramebuffer(VulcanImage& image);

        VulcanDevice& device;
        VulcanPool&   pool;
        Window&       window;

        VkDescriptorPool descriptorPool   = VK_NULL_HANDLE;
        VkRenderPass     renderPass       = VK_NULL_HANDLE;
        VkFormat         renderPassFormat = VK_FORMAT_UNDEFINED;

        std::unordered_map<VkImageView, VkFramebuffer> framebufferCache;
    };
}
