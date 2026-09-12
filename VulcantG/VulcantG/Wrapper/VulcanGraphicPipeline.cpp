#include "VulcanGraphicPipeline.h"
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rd_vertex_attribute.hpp>
#include <godot_cpp/classes/rd_pipeline_rasterization_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state_attachment.hpp>
#include <godot_cpp/classes/rd_pipeline_depth_stencil_state.hpp>
#include <godot_cpp/classes/rd_pipeline_multisample_state.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <stdexcept>
#include "VulcanDevice.h"

namespace Vulcant::VulcantG::Wrapper
{
    VulcanGraphicPipeline::VulcanGraphicPipeline(VulcanDevice& deviceInput, const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
      : device(deviceInput)
    {
        createFramebuffer(color, depth, stencil);
        createPipeline(shader, color, depth, stencil);
    }

    VulcanGraphicPipeline::~VulcanGraphicPipeline()
    {
        auto& rd = device.getDevice();
        if (pipeline.is_valid())
        {
            rd.free_rid(pipeline);
        }
        if (framebuffer.is_valid())
        {
            rd.free_rid(framebuffer);
        }
        if (shaderRid.is_valid())
        {
            rd.free_rid(shaderRid);
        }
    }

    godot::RID VulcanGraphicPipeline::getPipeline() const
    {
        return pipeline;
    }

    godot::RID VulcanGraphicPipeline::getFramebuffer() const
    {
        return framebuffer;
    }

    godot::RID VulcanGraphicPipeline::getShader() const
    {
        return shaderRid;
    }

    int64_t VulcanGraphicPipeline::getVertexFormat() const
    {
        return vertexFormat;
    }

    glm::uvec2 VulcanGraphicPipeline::getExtent() const
    {
        return extent;
    }

    void VulcanGraphicPipeline::createFramebuffer(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
    {
        godot::RenderingDevice& rd = device.getDevice();

        godot::TypedArray<godot::RID> attachments;
        for (auto* c : color)
        {
            if (c && c->getRid().is_valid())
            {
                attachments.push_back(c->getRid());
            }
        }
        if (depth && depth->getRid().is_valid())
        {
            attachments.push_back(depth->getRid());
        }

        if (!color.empty() && color[0])
        {
            extent = { color[0]->getWidth(), color[0]->getHeight() };
        }
        else if (depth)
        {
            extent = { depth->getWidth(), depth->getHeight() };
        }

        framebuffer = rd.framebuffer_create(attachments);
    }

    void VulcanGraphicPipeline::createPipeline(const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
    {
        godot::RenderingDevice& rd = device.getDevice();

        godot::Ref<godot::RDShaderSPIRV> rd_spirv;
        rd_spirv.instantiate();

        const VulcanShader* vertShader = nullptr;
        for (const auto* s : shader)
        {
            if (s)
            {
                rd_spirv->set_stage_bytecode(s->getStage(), s->getBytecode());
                if (s->getStage() == godot::RenderingDevice::SHADER_STAGE_VERTEX)
                {
                    vertShader = s;
                }
            }
        }

        auto binary = rd.shader_compile_binary_from_spirv(rd_spirv);
        shaderRid   = rd.shader_create_from_bytecode(binary);

        if (!shaderRid.is_valid())
        {
            for (const auto* s : shader)
            {
                if (s && s->getShader().is_valid())
                {
                    shaderRid = s->getShader();
                    break;
                }
            }
        }

        godot::TypedArray<godot::RDVertexAttribute> vertex_attributes;
        if (vertShader)
        {
            auto     layout = vertShader->getVertexLayout();
            uint32_t stride = 0;
            for (const auto& attr : layout)
            {
                stride += attr.size;
            }

            uint32_t currentOffset = 0;
            for (const auto& attr : layout)
            {
                godot::Ref<godot::RDVertexAttribute> rd_attr;
                rd_attr.instantiate();
                rd_attr->set_location(attr.location);
                rd_attr->set_offset(currentOffset);
                rd_attr->set_stride(stride);

                godot::RenderingDevice::DataFormat df = godot::RenderingDevice::DATA_FORMAT_R32G32B32A32_SFLOAT;
                switch (attr.format)
                {
                    case VK_FORMAT_R32_SFLOAT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32_SFLOAT;
                        break;
                    case VK_FORMAT_R32G32_SFLOAT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32G32_SFLOAT;
                        break;
                    case VK_FORMAT_R32G32B32_SFLOAT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32G32B32_SFLOAT;
                        break;
                    case VK_FORMAT_R32G32B32A32_SFLOAT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32G32B32A32_SFLOAT;
                        break;
                    case VK_FORMAT_R32_SINT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32_SINT;
                        break;
                    case VK_FORMAT_R32G32_SINT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32G32_SINT;
                        break;
                    case VK_FORMAT_R32G32B32_SINT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32G32B32_SINT;
                        break;
                    case VK_FORMAT_R32G32B32A32_SINT:
                        df = godot::RenderingDevice::DATA_FORMAT_R32G32B32A32_SINT;
                        break;
                    default:
                        df = static_cast<godot::RenderingDevice::DataFormat>(attr.format);
                        break;
                }
                rd_attr->set_format(df);
                rd_attr->set_frequency(attr.rate == Vulcant::Wrapper::InputRate::Instance ?
                                        godot::RenderingDevice::VERTEX_FREQUENCY_INSTANCE :
                                        godot::RenderingDevice::VERTEX_FREQUENCY_VERTEX);
                vertex_attributes.push_back(rd_attr);
                currentOffset += attr.size;
            }
        }
        vertexFormat = rd.vertex_format_create(vertex_attributes);

        int64_t framebufferFormat = rd.framebuffer_get_format(framebuffer);

        godot::Ref<godot::RDPipelineRasterizationState> rasterization_state;
        rasterization_state.instantiate();
        rasterization_state->set_cull_mode(godot::RenderingDevice::POLYGON_CULL_DISABLED);

        godot::Ref<godot::RDPipelineMultisampleState> multisample_state;
        multisample_state.instantiate();

        godot::Ref<godot::RDPipelineDepthStencilState> depth_stencil_state;
        depth_stencil_state.instantiate();
        if (depth && depth->getRid().is_valid())
        {
            depth_stencil_state->set_enable_depth_test(true);
            depth_stencil_state->set_enable_depth_write(true);
            depth_stencil_state->set_depth_compare_operator(godot::RenderingDevice::COMPARE_OP_LESS_OR_EQUAL);
        }
        else
        {
            depth_stencil_state->set_enable_depth_test(false);
            depth_stencil_state->set_enable_depth_write(false);
            depth_stencil_state->set_depth_compare_operator(godot::RenderingDevice::COMPARE_OP_ALWAYS);
        }

        godot::Ref<godot::RDPipelineColorBlendState> color_blend_state;
        color_blend_state.instantiate();
        godot::TypedArray<godot::Ref<godot::RDPipelineColorBlendStateAttachment>> blend_attachments;
        for (size_t i = 0; i < color.size(); ++i)
        {
            godot::Ref<godot::RDPipelineColorBlendStateAttachment> att;
            att.instantiate();
            att->set_enable_blend(false);
            att->set_write_r(true);
            att->set_write_g(true);
            att->set_write_b(true);
            att->set_write_a(true);
            blend_attachments.push_back(att);
        }
        color_blend_state->set_attachments(blend_attachments);

        pipeline = rd.render_pipeline_create(shaderRid,
                                             framebufferFormat,
                                             vertexFormat,
                                             godot::RenderingDevice::RENDER_PRIMITIVE_TRIANGLES,
                                             rasterization_state,
                                             multisample_state,
                                             depth_stencil_state,
                                             color_blend_state,
                                             0,
                                             0);
    }
}