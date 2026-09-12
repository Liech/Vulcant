#pragma once

#include "Vulcant/Interface/VulcantResourceLayout.h"
#include "VulcanBuffer.h"
#include "VulcanDevice.h"
#include "VulcanGraphicPipeline.h"
#include "VulcanImage.h"
#include "VulcanSet.h"
#include "VulcanShader.h"
#include <functional>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <vector>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanGraphicCommand
    {
      public:
        explicit VulcanGraphicCommand(VulcanDevice& dev);
        ~VulcanGraphicCommand();

        void beginRendering(VulcanGraphicPipeline& pipeline);
        void endRendering();

        void startRecord();
        void setViewportAndScissor(glm::uvec2 extent);
        void draw(uint32_t vertexCount, VulcanSet* set = nullptr, VulcanBuffer* vertexBuffer = nullptr);
        void add(uint32_t vertexCount, const VulcanSet& set, VulcanShader& shader, VulcanBuffer* vertexBuffer);
        void addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest);
        void addBarrier(VulcanBuffer& buffer);
        void queueTask(std::function<void()> task);
        void endRecord();

        void runAsync();
        void runSync();
        void wait();

        int64_t getDrawList() const;
        VulcanDevice& getDevice();

      private:
        VulcanDevice& device;
        bool          isRecording = false;
        bool          isSubmitted = false;
        int64_t       drawList    = 0;
        glm::uvec2    viewportExtent{ 0, 0 };

        VulcanGraphicPipeline* currentPipeline = nullptr;

        std::vector<std::function<void()>> beforeQueue;
        std::vector<std::function<void()>> drawListQueue;
        std::vector<std::function<void()>> afterQueue;
        std::vector<godot::RID>            createdVertexArrays;
    };
}