#pragma once

#include "VulcanGraphicPipeline.h"
#include "Library/Vulcant/Wrapper/VulcanBuffer.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanSet.h"
#include "Library/Vulcant/Wrapper/Helper/VulcanBarrier.h"
#include "Library/Vulcant/Wrapper/Helper/VulcanCommand.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Vulcant::Wrapper
{
    class VulcanGraphicCommand
    {
    public:
        enum class status { ready, started, inRenderPass, filled, ongpu };

        VulcanGraphicCommand(VulcanPool& poolInput, VulcanDevice& deviceInput);
        ~VulcanGraphicCommand();

        void startRecord();
        void endRecord();

        // 1. Render-Pass starten
        void beginRendering(VulcanGraphicPipeline& pipeline);
        void endRendering();

        // 2. Dynamic States setzen
        void setViewportAndScissor(glm::uvec2 extent);

        // 3. Render Pipeline & Draw Call
        void draw(uint32_t vertexCount, 
                  VulcanSet* set = nullptr, 
                  VulcanBuffer* vertexBuffer = nullptr);

        void addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest);
        void addBarrier(VulcanBuffer& buffer);

        void runAsync();
        void runSync();
        void wait();

        VkCommandBuffer __getCommandBuffer() { return cmd->getCommandBuffer(); }

    private:
        VulcanDevice& device;
        VulcanPool& pool;
        VulcanGraphicPipeline*         currentPipeline = nullptr;
        std::unique_ptr<VulcanCommand> cmd;
        std::unique_ptr<VulcanBarrier> barrier;
        status currentStatus = status::ready;
    };
}