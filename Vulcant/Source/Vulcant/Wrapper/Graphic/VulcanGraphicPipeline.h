#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanImage;
    class VulcanShader;

    class VulcanGraphicPipeline
    {
      public:
        VulcanGraphicPipeline(VulcanDevice&, const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth = nullptr, VulcanImage* stencil = nullptr);
        virtual ~VulcanGraphicPipeline();

        VkPipeline       getPipeline();
        VkPipelineLayout getLayout();
        VkRenderPass     getRenderPass();
        VkFramebuffer    getFramebuffer();
        glm::uvec2       getExtent() const;

      private:
        void createRenderPass(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);
        void createFramebuffer(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);
        void createPipeline(const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);

        VulcanDevice& device;

        VkPipeline       pipeline    = VK_NULL_HANDLE;
        VkPipelineLayout layout      = VK_NULL_HANDLE;
        VkRenderPass     renderPass  = VK_NULL_HANDLE;
        VkFramebuffer    framebuffer = VK_NULL_HANDLE;
        glm::uvec2       extent      = { 0, 0 };
    };
}