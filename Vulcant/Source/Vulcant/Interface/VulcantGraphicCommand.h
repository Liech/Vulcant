#pragma once

#include "VulcantResourceLayout.h"
#include <glm/glm.hpp>

namespace Vulcant
{
    class VulcantGraphicPipeline;
    class VulcantShader;
    class VulcantBuffer;
    class VulcantImage;
    class VulcantSet;

    class VulcantGraphicCommand
    {
      public:
        VulcantGraphicCommand()          = default;
        virtual ~VulcantGraphicCommand() = default;

        virtual void startRecord() = 0;
        virtual void endRecord()   = 0;

        virtual void beginRendering(VulcantGraphicPipeline& pipeline) = 0;
        virtual void endRendering()                                   = 0;

        virtual void setViewportAndScissor(glm::uvec2 extent) = 0;

        virtual void draw(uint32_t vertexCount, VulcantSet* set = nullptr, VulcantBuffer* vertexBuffer = nullptr) = 0;

        virtual void addBarrier(VulcantImage& inputImg, const VulcantResourceLayout& dest) = 0;
        virtual void addBarrier(VulcantBuffer& buffer)                                     = 0;

        virtual void runAsync() = 0;
        virtual void runSync()  = 0;
        virtual void wait()     = 0;
    };
}
