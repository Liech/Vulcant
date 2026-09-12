#include "VulcanGraphicCommand.h"
#include <cassert>
#include <iostream>

namespace Vulcant::VulcantG::Wrapper
{
    VulcanGraphicCommand::VulcanGraphicCommand(VulcanDevice& dev)
      : device(dev)
    {
    }

    VulcanGraphicCommand::~VulcanGraphicCommand()
    {
        wait(); // Ensure GPU execution completes before destroying recorded RIDs

        if (isRecording)
            endRecord();

        auto& rd = device.getDevice();
        for (auto& va : createdVertexArrays)
        {
            if (va.is_valid())
            {
                rd.free_rid(va);
            }
        }
        createdVertexArrays.clear();
    }

    void VulcanGraphicCommand::startRecord()
    {
        wait(); // Ensure previous frame submission finished on GPU

        auto& rd = device.getDevice();
        for (auto& va : createdVertexArrays)
        {
            if (va.is_valid())
            {
                rd.free_rid(va);
            }
        }
        createdVertexArrays.clear();

        beforeQueue.clear();
        drawListQueue.clear();
        afterQueue.clear();

        drawListQueue.push_back([this]() { device.getDevice().capture_timestamp("VulcanGraphicCommand::startRecord"); });
        isRecording    = true;
        isSubmitted    = false;
        drawList       = 0;
        viewportExtent = { 0, 0 };
    }

    void VulcanGraphicCommand::beginRendering(VulcanGraphicPipeline& pipeline)
    {
        assert(isRecording);
        currentPipeline = &pipeline;

        auto* pipe = currentPipeline;

        // Queue opening the render pass at execution time
        drawListQueue.push_back(
          [this, pipe]()
          {
              auto& rd     = device.getDevice();
              auto  extent = (viewportExtent.x > 0 && viewportExtent.y > 0) ? viewportExtent : pipe->getExtent();

              godot::PackedColorArray clear_colors;
              clear_colors.push_back(godot::Color(0.0f, 0.0f, 0.0f, 1.0f));

              godot::Rect2 region(0, 0, extent.x, extent.y);

              drawList = rd.draw_list_begin(pipe->getFramebuffer(), godot::RenderingDevice::DRAW_DEFAULT_ALL, clear_colors, 1.0f, 0, region, 0);
          });
    }

    void VulcanGraphicCommand::setViewportAndScissor(glm::uvec2 extent)
    {
        viewportExtent = extent;
        drawListQueue.push_back(
          [this, extent]()
          {
              if (drawList != 0)
              {
                  device.getDevice().draw_list_enable_scissor(drawList, godot::Rect2(0, 0, extent.x, extent.y));
              }
          });
    }

    void VulcanGraphicCommand::draw(uint32_t vertexCount, VulcanSet* set, VulcanBuffer* vertexBuffer)
    {
        assert(currentPipeline);
        auto* pipe = currentPipeline;

        // 1. Synchronously create the Vertex Array RID during record step
        godot::RID vertexArray;
        if (vertexBuffer && vertexBuffer->getRid().is_valid())
        {
            godot::TypedArray<godot::RID> src_buffers;
            godot::PackedInt64Array       offsets;

            // Supply the vertexBuffer RID once for EVERY attribute registered in the vertex format layout
            uint32_t attrCount = pipe->getVertexAttributeCount();
            for (uint32_t i = 0; i < attrCount; ++i)
            {
                src_buffers.push_back(vertexBuffer->getRid());
                offsets.push_back(0);
            }

            vertexArray = device.getDevice().vertex_array_create(vertexCount, pipe->getVertexFormat(), src_buffers, offsets);

            if (vertexArray.is_valid())
            {
                createdVertexArrays.push_back(vertexArray);
            }
            else
            {
                std::cerr << "[VulcanGraphicCommand] vertex_array_create failed!" << std::endl;
            }
        }

        // 2. Queue the draw call actions during playback step
        drawListQueue.push_back(
          [this, pipe, vertexCount, set, vertexArray]()
          {
              if (drawList == 0)
                  return;

              auto& rd = device.getDevice();

              rd.draw_list_bind_render_pipeline(drawList, pipe->getPipeline());

              if (set)
              {
                  set->bindDrawList(drawList);
              }

              if (vertexArray.is_valid())
              {
                  rd.draw_list_bind_vertex_array(drawList, vertexArray);
              }

              rd.draw_list_draw(drawList, false, 1, vertexCount);
          });
    }

    void VulcanGraphicCommand::endRendering()
    {
        drawListQueue.push_back(
          [this]()
          {
              if (drawList != 0)
              {
                  device.getDevice().draw_list_end();
                  drawList = 0;
              }
          });
        currentPipeline = nullptr;
    }

    void VulcanGraphicCommand::endRecord()
    {
        assert(drawListQueue.size() > 0);
        drawListQueue.push_back([this]() { device.getDevice().capture_timestamp("VulcanGraphicCommand::endRecord"); });
        isRecording = false;
    }

    void VulcanGraphicCommand::add(uint32_t vertexCount, const VulcanSet& set, VulcanShader& shader, VulcanBuffer* vertexBuffer)
    {
        draw(vertexCount, const_cast<VulcanSet*>(&set), vertexBuffer);
    }

    void VulcanGraphicCommand::addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest)
    {
        drawListQueue.push_back([this]() { device.getDevice().full_barrier(); });
    }

    void VulcanGraphicCommand::addBarrier(VulcanBuffer& buffer)
    {
        drawListQueue.push_back([this]() { device.getDevice().full_barrier(); });
    }

    void VulcanGraphicCommand::queueTask(std::function<void()> task)
    {
        drawListQueue.push_back(std::move(task));
    }

    void VulcanGraphicCommand::runAsync()
    {
        assert(!isRecording && drawListQueue.size() > 0);

        for (const auto& task : beforeQueue)
            task();
        for (const auto& task : drawListQueue)
            task();
        for (const auto& task : afterQueue)
            task();

        if (!device.isGlobal())
        {
            device.getDevice().submit();
        }
        isSubmitted = true;
    }

    void VulcanGraphicCommand::runSync()
    {
        runAsync();
        wait();
    }

    void VulcanGraphicCommand::wait()
    {
        if (isSubmitted)
        {
            if (!device.isGlobal())
            {
                device.getDevice().sync();
            }
            isSubmitted = false;
        }
    }

    int64_t VulcanGraphicCommand::getDrawList() const
    {
        return drawList;
    }

    VulcanDevice& VulcanGraphicCommand::getDevice()
    {
        return device;
    }
}