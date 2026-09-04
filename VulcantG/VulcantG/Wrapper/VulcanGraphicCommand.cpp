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
        if (isRecording)
            endRecord();
    }

    void VulcanGraphicCommand::beginRendering(VulcanGraphicPipeline& pipeline) { currentPipeline = &pipeline; }
    void VulcanGraphicCommand::endRendering()
    {
        currentPipeline = nullptr;
    }


    void VulcanGraphicCommand::startRecord()
    {
        assert(drawListQueue.size() == 0);
        assert(!isSubmitted);
        assert(!isRecording);

        drawListQueue.push_back([this]() { device.getDevice().capture_timestamp("VulcanGraphicCommand::startRecord"); });
        isRecording = true;
        isSubmitted = false;
    }

    void VulcanGraphicCommand::add(uint32_t vertexCount, const VulcanSet& set, VulcanShader& shader, VulcanBuffer* vertexBuffer)
    {
        assert(currentPipeline);

        drawListQueue.push_back(
          [this, vertexCount, &set, &shader, vertexBuffer]()
          {
              auto& rd = device.getDevice();

              if (drawList == 0)
              {
                  godot::PackedColorArray clear_colors;
                  clear_colors.push_back(godot::Color(0.1f, 0.1f, 0.1f, 1.0f));

                  auto         extent = currentPipeline->getExtent();
                  godot::Rect2 region(0, 0, extent.x, extent.y);

                  drawList = rd.draw_list_begin(currentPipeline->getFramebuffer(),
                                                godot::RenderingDevice::DRAW_DEFAULT_ALL,
                                                clear_colors,
                                                1.0f, // Clear Depth
                                                0,    // Clear Stencil
                                                region,
                                                0 // Breadcrumb
                  );
              }

              rd.draw_list_bind_render_pipeline(drawList, currentPipeline->getPipeline());
              shader.bind(drawList);
              set.bind(drawList);

              if (vertexBuffer && vertexBuffer->getRid().is_valid())
              {
                  rd.draw_list_bind_vertex_array(drawList, vertexBuffer->getRid());
              }

              rd.draw_list_draw(drawList, false, 1, vertexCount);
          });
    }

    void VulcanGraphicCommand::endRecord()
    {
        assert(drawListQueue.size() > 0);
        drawListQueue.push_back(
          [this]()
          {
              if (drawList != 0)
              {
                  device.getDevice().draw_list_end();
                  drawList = 0;
              }
              device.getDevice().capture_timestamp("VulcanGraphicCommand::endRecord");
          });
        isRecording = false;
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
        assert(isSubmitted);
        if (!device.isGlobal())
        {
            device.getDevice().sync();
        }
        isSubmitted = false;
    }

    int64_t VulcanGraphicCommand::getDrawList() const
    {
        return drawList;
    }
}