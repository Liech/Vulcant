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
        VulcanUi(VulcanDevice& device, VulcanPool& pool, Window& window, VkFormat formatInput = VK_FORMAT_R8G8B8A8_UNORM, bool clear = false);
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
        bool             clear            = false;

        std::unordered_map<VkImageView, VkFramebuffer> framebufferCache;
    };
}
