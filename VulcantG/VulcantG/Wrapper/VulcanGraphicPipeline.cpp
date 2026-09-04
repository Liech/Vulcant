#include "VulcanGraphicPipeline.h"
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rd_vertex_attribute.hpp>
#include <godot_cpp/classes/rd_pipeline_rasterization_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state.hpp>
#include <godot_cpp/classes/rd_pipeline_depth_stencil_state.hpp>
#include <godot_cpp/classes/rd_pipeline_multisample_state.hpp>
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
    }

    godot::RID VulcanGraphicPipeline::getPipeline() const
    {
        return pipeline;
    }

    godot::RID VulcanGraphicPipeline::getFramebuffer() const
    {
        return framebuffer;
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

        framebuffer = rd.framebuffer_create(attachments);
    }

    void VulcanGraphicPipeline::createPipeline(const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil)
    {
        godot::RenderingDevice& rd = device.getDevice();

        godot::RID shaderRid;
        for (const auto* s : shader)
        {
            if (s)
            {
                shaderRid = s->getShader();
                break;
            }
        }

        godot::TypedArray<godot::RDVertexAttribute> vertex_attributes;
        int64_t                                     vertexFormat = rd.vertex_format_create(vertex_attributes);

        int64_t framebufferFormat = rd.framebuffer_get_format(framebuffer);

        godot::Ref<godot::RDPipelineRasterizationState> rasterization_state;
        rasterization_state.instantiate();

        godot::Ref<godot::RDPipelineMultisampleState> multisample_state;
        godot::Ref<godot::RDPipelineDepthStencilState> depth_stencil_state;
        godot::Ref<godot::RDPipelineColorBlendState> color_blend_state;

        pipeline = rd.render_pipeline_create(shaderRid,
                                             framebufferFormat,
                                             vertexFormat,
                                             godot::RenderingDevice::RENDER_PRIMITIVE_TRIANGLES,
                                             rasterization_state,
                                             multisample_state,
                                             depth_stencil_state,
                                             color_blend_state,
                                             0, // dynamic state flags
                                             0  // for render pass
        );
    }
}