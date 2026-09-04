#pragma once

#include "Library/Vulcant/Interface/VulcantGraphicCommand.h"
#include <memory>

namespace Vulcant::Wrapper
{
    class VulcanGraphicCommand;
    class VulcanPool;
    class VulcanDevice;
    class VulcanShader;
    class VulcanSet;
}

namespace Vulcant::VulcantV
{
    class VulcantVGraphicCommand : public VulcantGraphicCommand
    {
      public:
        VulcantVGraphicCommand(Wrapper::VulcanPool& pool, Wrapper::VulcanDevice& device);
        virtual ~VulcantVGraphicCommand();

        virtual void startRecord() override;
        virtual void endRecord() override;

        virtual void beginRendering(Vulcant::VulcantGraphicPipeline& pipeline) override;
        virtual void endRendering() override;

        virtual void setViewportAndScissor(glm::uvec2 extent) override;

        virtual void draw(uint32_t vertexCount, VulcantSet* set = nullptr, VulcantBuffer* vertexBuffer = nullptr) override;

        virtual void addBarrier(VulcantImage& inputImg, const VulcantResourceLayout& dest) override;
        virtual void addBarrier(VulcantBuffer& buffer) override;

        virtual void runAsync() override;
        virtual void runSync() override;
        virtual void wait() override;

        std::unique_ptr<Wrapper::VulcanGraphicCommand> cmd;
    };
}
