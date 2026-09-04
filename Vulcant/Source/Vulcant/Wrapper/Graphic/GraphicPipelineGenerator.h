#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <optional>
#include <vector>

namespace Vulcant::Wrapper
{
    class VulcanImage;
    class VulcanShader;
    struct ShaderBindingDefinition;
}

namespace Vulcant::Wrapper::Graphic
{
    // This class contains the boilerplate part oft the pipeline initialization for Graphics Pipelines.
    // interesting options should be exposed if needed
    class GraphicPipelineGenerator
    {
      public:
        GraphicPipelineGenerator();
        virtual ~GraphicPipelineGenerator();

        void setBoiler();
        void setColor(const std::vector<VulcanImage*>& color);
        void setDepthStencil(VulcanImage* depth = nullptr, VulcanImage* stencil = nullptr);
        void setShader(const std::vector<VulcanShader*>& shader);
        void setRenderPass(VkRenderPass renderPass, uint32_t subpass = 0);

        VkGraphicsPipelineCreateInfo& get();

      private:
        void fillMultisample();
        void fillDepth();
        void fillRasterizer();
        void fillMisc();
        void fillVertex(VulcanShader&);


        VkGraphicsPipelineCreateInfo                     pipeInfo{};
        VkPipelineMultisampleStateCreateInfo             multisampling{};
        VkPipelineDepthStencilStateCreateInfo            depthStencil{};
        std::vector<VkDynamicState>                      dynamicStates;
        VkPipelineViewportStateCreateInfo                viewportState{};
        VkPipelineDynamicStateCreateInfo                 dynamicState{};
        VkPipelineRasterizationStateCreateInfo           rasterizer{};
        VkPipelineInputAssemblyStateCreateInfo           inputAssembly{};
        std::vector<VkFormat>                            colorFormats;
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
        VkPipelineColorBlendStateCreateInfo              colorBlending{};
        std::vector<VkPipelineShaderStageCreateInfo>     shaderStages;

        // vertex
        VkPipelineVertexInputStateCreateInfo           vertexInputInfo{};
        VkVertexInputBindingDescription                bindingDescription{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    };
}