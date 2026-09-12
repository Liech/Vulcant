#include "VulcanUi.h"

#include "VulcanDevice.h"
#include "VulcanImage.h"
#include "VulcanGraphicCommand.h"
#include "VulcantG/VulcantG/VulcantGWindow.h"
#include "Vulcant/Interface/VulcantInput.h"
#include "Vulcant/Interface/VulcantInputValue.h"

#include <imgui.h>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/rd_sampler_state.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rd_vertex_attribute.hpp>
#include <godot_cpp/classes/rd_pipeline_rasterization_state.hpp>
#include <godot_cpp/classes/rd_pipeline_multisample_state.hpp>
#include <godot_cpp/classes/rd_pipeline_depth_stencil_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state_attachment.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/rect2.hpp>

#include <cstring>
#include <algorithm>

namespace Vulcant::VulcantG::Wrapper
{
    VulcanUi::VulcanUi(VulcanDevice& dev, VulcantGWindow* win)
      : device(dev)
      , window(win)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.BackendRendererName = "imgui_impl_godot_rd";

        ImGui::StyleColorsDark();

        initPipeline();
        initFontTexture();

        lastFrameTime = std::chrono::steady_clock::now();
    }

    VulcanUi::~VulcanUi()
    {
        cleanupFrameResources();

        auto& rd = device.getDevice();
        if (fontUniformSet.is_valid())
        {
            rd.free_rid(fontUniformSet);
        }
        if (fontTexture.is_valid())
        {
            rd.free_rid(fontTexture);
        }
        if (fontSampler.is_valid())
        {
            rd.free_rid(fontSampler);
        }
        if (pipeline.is_valid())
        {
            rd.free_rid(pipeline);
        }
        if (shaderRid.is_valid())
        {
            rd.free_rid(shaderRid);
        }

        for (auto& [rid, fb] : framebufferCache)
        {
            if (fb.is_valid())
            {
                rd.free_rid(fb);
            }
        }
        framebufferCache.clear();

        ImGui::DestroyContext();
    }

    void VulcanUi::cleanupFrameResources()
    {
        auto& rd = device.getDevice();
        for (auto& rid : frameBuffersToFree)
        {
            if (rid.is_valid())
            {
                rd.free_rid(rid);
            }
        }
        frameBuffersToFree.clear();
    }

    void VulcanUi::initPipeline()
    {
        auto& rd = device.getDevice();

        godot::Ref<godot::RDShaderSource> shader_source;
        shader_source.instantiate();

        shader_source->set_stage_source(godot::RenderingDevice::SHADER_STAGE_VERTEX,
            "#version 450\n"
            "layout(location = 0) in vec2 aPos;\n"
            "layout(location = 1) in vec2 aUV;\n"
            "layout(location = 2) in vec4 aColor;\n"
            "layout(push_constant) uniform PushConstants {\n"
            "    vec2 uScale;\n"
            "    vec2 uTranslate;\n"
            "} pc;\n"
            "layout(location = 0) out vec2 vUV;\n"
            "layout(location = 1) out vec4 vColor;\n"
            "void main() {\n"
            "    vUV = aUV;\n"
            "    vColor = aColor;\n"
            "    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0.0, 1.0);\n"
            "}\n");

        shader_source->set_stage_source(godot::RenderingDevice::SHADER_STAGE_FRAGMENT,
            "#version 450\n"
            "layout(location = 0) in vec2 vUV;\n"
            "layout(location = 1) in vec4 vColor;\n"
            "layout(binding = 0) uniform sampler2D sTexture;\n"
            "layout(location = 0) out vec4 fColor;\n"
            "void main() {\n"
            "    fColor = vColor * texture(sTexture, vUV);\n"
            "}\n");

        auto spirv  = rd.shader_compile_spirv_from_source(shader_source);
        auto binary = rd.shader_compile_binary_from_spirv(spirv);
        shaderRid   = rd.shader_create_from_bytecode(binary);

        godot::TypedArray<godot::RDVertexAttribute> vertex_attrs;

        godot::Ref<godot::RDVertexAttribute> attr_pos;
        attr_pos.instantiate();
        attr_pos->set_location(0);
        attr_pos->set_offset(offsetof(ImDrawVert, pos));
        attr_pos->set_format(godot::RenderingDevice::DATA_FORMAT_R32G32_SFLOAT);
        attr_pos->set_stride(sizeof(ImDrawVert));
        attr_pos->set_frequency(godot::RenderingDevice::VERTEX_FREQUENCY_VERTEX);
        vertex_attrs.push_back(attr_pos);

        godot::Ref<godot::RDVertexAttribute> attr_uv;
        attr_uv.instantiate();
        attr_uv->set_location(1);
        attr_uv->set_offset(offsetof(ImDrawVert, uv));
        attr_uv->set_format(godot::RenderingDevice::DATA_FORMAT_R32G32_SFLOAT);
        attr_uv->set_stride(sizeof(ImDrawVert));
        attr_uv->set_frequency(godot::RenderingDevice::VERTEX_FREQUENCY_VERTEX);
        vertex_attrs.push_back(attr_uv);

        godot::Ref<godot::RDVertexAttribute> attr_col;
        attr_col.instantiate();
        attr_col->set_location(2);
        attr_col->set_offset(offsetof(ImDrawVert, col));
        attr_col->set_format(godot::RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM);
        attr_col->set_stride(sizeof(ImDrawVert));
        attr_col->set_frequency(godot::RenderingDevice::VERTEX_FREQUENCY_VERTEX);
        vertex_attrs.push_back(attr_col);

        vertexFormat = rd.vertex_format_create(vertex_attrs);
    }

    void VulcanUi::initFontTexture()
    {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        auto& rd = device.getDevice();

        godot::Ref<godot::RDTextureFormat> tf;
        tf.instantiate();
        tf->set_format(godot::RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM);
        tf->set_texture_type(godot::RenderingDevice::TEXTURE_TYPE_2D);
        tf->set_width(width);
        tf->set_height(height);
        tf->set_depth(1);
        tf->set_array_layers(1);
        tf->set_mipmaps(1);
        tf->set_usage_bits(godot::RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | godot::RenderingDevice::TEXTURE_USAGE_CAN_UPDATE_BIT);

        godot::PackedByteArray data;
        data.resize(width * height * 4);
        std::memcpy(data.ptrw(), pixels, width * height * 4);

        godot::TypedArray<godot::PackedByteArray> data_array;
        data_array.push_back(data);

        godot::Ref<godot::RDTextureView> tv;
        tv.instantiate();

        fontTexture = rd.texture_create(tf, tv, data_array);

        godot::Ref<godot::RDSamplerState> ss;
        ss.instantiate();
        ss->set_min_filter(godot::RenderingDevice::SAMPLER_FILTER_LINEAR);
        ss->set_mag_filter(godot::RenderingDevice::SAMPLER_FILTER_LINEAR);
        ss->set_repeat_u(godot::RenderingDevice::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
        ss->set_repeat_v(godot::RenderingDevice::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
        ss->set_repeat_w(godot::RenderingDevice::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
        fontSampler = rd.sampler_create(ss);

        godot::TypedArray<godot::RDUniform> uniforms;
        godot::Ref<godot::RDUniform> u;
        u.instantiate();
        u->set_uniform_type(godot::RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
        u->set_binding(0);
        u->add_id(fontSampler);
        u->add_id(fontTexture);
        uniforms.append(u);

        fontUniformSet = rd.uniform_set_create(uniforms, shaderRid, 0);

        io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(fontUniformSet.get_id())));
    }

    godot::RID VulcanUi::getOrCreateFramebuffer(VulcanImage& targetImage)
    {
        auto& rd  = device.getDevice();
        uint64_t key = targetImage.getRid().get_id();
        auto it   = framebufferCache.find(key);
        if (it != framebufferCache.end() && it->second.is_valid())
        {
            return it->second;
        }

        godot::TypedArray<godot::RID> attachments;
        attachments.push_back(targetImage.getRid());
        godot::RID fb = rd.framebuffer_create(attachments);
        framebufferCache[key] = fb;
        return fb;
    }

    void VulcanUi::newFrame()
    {
        ImGuiIO& io = ImGui::GetIO();

        if (window)
        {
            glm::ivec2 res = window->getResolution();
            io.DisplaySize = ImVec2(static_cast<float>(res.x), static_cast<float>(res.y));

            glm::dvec2 mPos = window->getMousePosition();
            io.AddMousePosEvent(static_cast<float>(mPos.x), static_cast<float>(res.y - mPos.y));

            auto& input = window->getInput();
            io.AddMouseButtonEvent(0, input.isDown(Vulcant::VulcantInputValue::MouseLeft));
            io.AddMouseButtonEvent(1, input.isDown(Vulcant::VulcantInputValue::MouseRight));
            io.AddMouseButtonEvent(2, input.isDown(Vulcant::VulcantInputValue::MouseMiddle));
        }

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastFrameTime;
        float dt = elapsed.count();
        io.DeltaTime  = dt > 0.0f ? dt : (1.0f / 60.0f);
        lastFrameTime = currentTime;

        ImGui::NewFrame();
    }

    void VulcanUi::render(VulcanGraphicCommand& cmd, VulcanImage& targetImage)
    {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (!draw_data || draw_data->TotalVtxCount == 0 || draw_data->CmdListsCount == 0)
        {
            return;
        }

        auto& rd = device.getDevice();
        godot::RID fb = getOrCreateFramebuffer(targetImage);

        int64_t fbFormat = rd.framebuffer_get_format(fb);
        if (!pipeline.is_valid() || pipelineFramebufferFormat != fbFormat)
        {
            if (pipeline.is_valid())
            {
                rd.free_rid(pipeline);
            }

            godot::Ref<godot::RDPipelineRasterizationState> raster_state;
            raster_state.instantiate();
            raster_state->set_cull_mode(godot::RenderingDevice::POLYGON_CULL_DISABLED);

            godot::Ref<godot::RDPipelineMultisampleState> ms_state;
            ms_state.instantiate();

            godot::Ref<godot::RDPipelineDepthStencilState> ds_state;
            ds_state.instantiate();
            ds_state->set_enable_depth_test(false);
            ds_state->set_enable_depth_write(false);

            godot::Ref<godot::RDPipelineColorBlendState> blend_state;
            blend_state.instantiate();
            godot::TypedArray<godot::Ref<godot::RDPipelineColorBlendStateAttachment>> blend_attachments;
            godot::Ref<godot::RDPipelineColorBlendStateAttachment> blend_att;
            blend_att.instantiate();
            blend_att->set_enable_blend(true);
            blend_att->set_src_color_blend_factor(godot::RenderingDevice::BLEND_FACTOR_SRC_ALPHA);
            blend_att->set_dst_color_blend_factor(godot::RenderingDevice::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
            blend_att->set_color_blend_op(godot::RenderingDevice::BLEND_OP_ADD);
            blend_att->set_src_alpha_blend_factor(godot::RenderingDevice::BLEND_FACTOR_ONE);
            blend_att->set_dst_alpha_blend_factor(godot::RenderingDevice::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
            blend_att->set_alpha_blend_op(godot::RenderingDevice::BLEND_OP_ADD);
            blend_att->set_write_r(true);
            blend_att->set_write_g(true);
            blend_att->set_write_b(true);
            blend_att->set_write_a(true);
            blend_attachments.push_back(blend_att);
            blend_state->set_attachments(blend_attachments);

            pipeline = rd.render_pipeline_create(shaderRid,
                                                 fbFormat,
                                                 vertexFormat,
                                                 godot::RenderingDevice::RENDER_PRIMITIVE_TRIANGLES,
                                                 raster_state,
                                                 ms_state,
                                                 ds_state,
                                                 blend_state);
            pipelineFramebufferFormat = fbFormat;
        }

        float scaleX = 2.0f / draw_data->DisplaySize.x;
        float scaleY = 2.0f / draw_data->DisplaySize.y;
        float transX = -1.0f - draw_data->DisplayPos.x * scaleX;
        float transY = -1.0f - draw_data->DisplayPos.y * scaleY;

        godot::PackedByteArray pc_data;
        pc_data.resize(sizeof(float) * 4);
        float* pc_floats = reinterpret_cast<float*>(pc_data.ptrw());
        pc_floats[0] = scaleX;
        pc_floats[1] = scaleY;
        pc_floats[2] = transX;
        pc_floats[3] = transY;

        struct DrawBatch
        {
            godot::Rect2 clipRect;
            godot::RID   uniformSet;
            uint32_t     vtxOffset;
            uint32_t     vtxCount;
        };

        std::vector<ImDrawVert> unrolledVertices;
        std::vector<DrawBatch>  batches;

        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->ElemCount == 0)
                    continue;

                DrawBatch batch;
                batch.vtxOffset = static_cast<uint32_t>(unrolledVertices.size());
                batch.vtxCount  = pcmd->ElemCount;

                godot::RID texRid;
                if (pcmd->TextureId)
                {
                    texRid = godot::RID::from_uint64(reinterpret_cast<uintptr_t>(pcmd->TextureId));
                }
                batch.uniformSet = texRid.is_valid() ? texRid : fontUniformSet;

                ImVec2 clip_min(pcmd->ClipRect.x - draw_data->DisplayPos.x, pcmd->ClipRect.y - draw_data->DisplayPos.y);
                ImVec2 clip_max(pcmd->ClipRect.z - draw_data->DisplayPos.x, pcmd->ClipRect.w - draw_data->DisplayPos.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                batch.clipRect = godot::Rect2(clip_min.x, clip_min.y, clip_max.x - clip_min.x, clip_max.y - clip_min.y);

                unrolledVertices.reserve(unrolledVertices.size() + pcmd->ElemCount);
                for (unsigned int i = 0; i < pcmd->ElemCount; i++)
                {
                    ImDrawIdx idx = cmd_list->IdxBuffer[pcmd->IdxOffset + i];
                    const ImDrawVert& v = cmd_list->VtxBuffer[pcmd->VtxOffset + idx];
                    unrolledVertices.push_back(v);
                }

                batches.push_back(batch);
            }
        }

        if (unrolledVertices.empty())
            return;

        cleanupFrameResources();

        size_t totalByteSize = unrolledVertices.size() * sizeof(ImDrawVert);
        godot::PackedByteArray vtx_bytes;
        vtx_bytes.resize(totalByteSize);
        std::memcpy(vtx_bytes.ptrw(), unrolledVertices.data(), totalByteSize);

        godot::RID vtxBuffer = rd.vertex_buffer_create(static_cast<uint32_t>(totalByteSize), vtx_bytes);
        frameBuffersToFree.push_back(vtxBuffer);

        std::vector<godot::RID> batchVertexArrays;
        batchVertexArrays.reserve(batches.size());
        for (const auto& b : batches)
        {
            godot::TypedArray<godot::RID> src_buffers;
            src_buffers.push_back(vtxBuffer);
            godot::PackedInt64Array offsets;
            offsets.push_back(b.vtxOffset * sizeof(ImDrawVert));

            godot::RID va = rd.vertex_array_create(b.vtxCount, vertexFormat, src_buffers, offsets);
            frameBuffersToFree.push_back(va);
            batchVertexArrays.push_back(va);
        }

        auto targetExtent = glm::uvec2(targetImage.getWidth(), targetImage.getHeight());
        godot::RID pipelineToUse = pipeline;

        cmd.queueTask(
          [this, fb, targetExtent, pipelineToUse, pc_data, batches, batchVertexArrays]()
          {
              auto& rd = device.getDevice();

              godot::PackedColorArray clear_colors;
              clear_colors.push_back(godot::Color(0.1f, 0.1f, 0.15f, 1.0f));

              godot::Rect2 region(0, 0, targetExtent.x, targetExtent.y);
              int64_t drawList = rd.draw_list_begin(fb,
                                                    godot::RenderingDevice::DRAW_DEFAULT_ALL,
                                                    clear_colors,
                                                    1.0f,
                                                    0,
                                                    region,
                                                    0);

              rd.draw_list_bind_render_pipeline(drawList, pipelineToUse);
              rd.draw_list_set_push_constant(drawList, pc_data, sizeof(float) * 4);

              for (size_t i = 0; i < batches.size(); i++)
              {
                  const auto& b = batches[i];
                  rd.draw_list_bind_uniform_set(drawList, b.uniformSet, 0);
                  rd.draw_list_enable_scissor(drawList, b.clipRect);
                  rd.draw_list_bind_vertex_array(drawList, batchVertexArrays[i]);
                  rd.draw_list_draw(drawList, false, 1, b.vtxCount);
              }

              rd.draw_list_end();
          });
    }
}
