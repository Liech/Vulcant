#include "GraphicPipelineGenerator.h"

#include "Vulcant/Wrapper/ShaderCompiler.h"
#include "Vulcant/Wrapper/VulcanImage.h"
#include "Vulcant/Wrapper/VulcanShader.h"
#include <cassert>
#include <stdexcept>

namespace Vulcant::Wrapper::Graphic
{
    GraphicPipelineGenerator::GraphicPipelineGenerator()
    {
        pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeInfo.pNext = nullptr;
    }

    GraphicPipelineGenerator::~GraphicPipelineGenerator() {}

    void GraphicPipelineGenerator::fillMultisample()
    {
        multisampling                      = {};
        multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading     = 1.0f;
    }

    void GraphicPipelineGenerator::fillDepth()
    {
        depthStencil                       = {};
        depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable       = VK_TRUE;
        depthStencil.depthWriteEnable      = VK_TRUE;
        depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable     = VK_FALSE;
        depthStencil.minDepthBounds        = 0.0f;
        depthStencil.maxDepthBounds        = 1.0f;
    }

    void GraphicPipelineGenerator::fillRasterizer()
    {
        rasterizer                         = {};
        rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = VK_CULL_MODE_NONE;
        rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable         = VK_FALSE;
    }

    void GraphicPipelineGenerator::fillMisc()
    {
        dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        dynamicState                   = {};
        dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates    = dynamicStates.data();

        viewportState               = {};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount  = 1;

        inputAssembly                        = {};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
    }

    void GraphicPipelineGenerator::setColor(const std::vector<VulcanImage*>& color)
    {
        colorFormats.clear();
        blendAttachments.clear();

        for (const auto& x : color)
        {
            colorFormats.push_back(x->getFormat());

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable    = VK_FALSE;
            blendAttachments.push_back(colorBlendAttachment);
        }

        colorBlending                 = {};
        colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable   = VK_FALSE;
        colorBlending.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
        colorBlending.pAttachments    = blendAttachments.data();

        pipeInfo.pColorBlendState = &colorBlending;
    }

    void GraphicPipelineGenerator::setDepthStencil(VulcanImage* depth, VulcanImage* stencil)
    {
    }

    void GraphicPipelineGenerator::setRenderPass(VkRenderPass renderPass, uint32_t subpass)
    {
        pipeInfo.renderPass = renderPass;
        pipeInfo.subpass    = subpass;
    }

    void GraphicPipelineGenerator::setBoiler()
    {
        fillMultisample();
        fillDepth();
        fillRasterizer();
        fillMisc();

        pipeInfo.pMultisampleState   = &multisampling;
        pipeInfo.pDepthStencilState  = &depthStencil;
        pipeInfo.pDynamicState       = &dynamicState;
        pipeInfo.pViewportState      = &viewportState;
        pipeInfo.pRasterizationState = &rasterizer;
        pipeInfo.pInputAssemblyState = &inputAssembly;
        pipeInfo.renderPass          = VK_NULL_HANDLE;
        pipeInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipeInfo.basePipelineIndex   = -1;
    }

    void GraphicPipelineGenerator::setShader(const std::vector<VulcanShader*>& shader)
    {
        assert(!shader.empty());
        pipeInfo.layout = shader[0]->getPipelineLayout();

        shaderStages.clear();
        for (auto* s : shader)
        {
            shaderStages.push_back(s->getCreateInfo());
        }

        pipeInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipeInfo.pStages    = shaderStages.data();

        bool foundVertex = false;
        for (auto* s : shader)
        {
            if (s->getStage() == VK_SHADER_STAGE_VERTEX_BIT)
            {
                fillVertex(*s);
                foundVertex = true;
                break;
            }
        }
        if (!foundVertex)
        {
            fillVertex(*shader[0]);
        }
    }

    void GraphicPipelineGenerator::fillVertex(VulcanShader& shader)
    {
        std::vector<VertexAttribute> layout = shader.getVertexLayout();

        size_t stride = 0;
        for (const auto& x : layout)
            stride += x.size;

        bindingDescription           = {};
        bindingDescription.binding   = 0;
        bindingDescription.stride    = static_cast<uint32_t>(stride);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        size_t currentOffset = 0;
        attributeDescriptions.resize(layout.size());
        for (size_t i = 0; i < layout.size(); i++)
        {
            const auto& x                     = layout[i];
            attributeDescriptions[i].binding  = 0;
            attributeDescriptions[i].location = x.location;
            attributeDescriptions[i].format   = x.format;
            attributeDescriptions[i].offset   = static_cast<uint32_t>(currentOffset);
            currentOffset += x.size;
        }

        vertexInputInfo                                 = {};
        vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount   = 1;
        vertexInputInfo.pVertexBindingDescriptions      = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

        pipeInfo.pVertexInputState = &vertexInputInfo;
    }

    VkGraphicsPipelineCreateInfo& GraphicPipelineGenerator::get()
    {
        return pipeInfo;
    }
}