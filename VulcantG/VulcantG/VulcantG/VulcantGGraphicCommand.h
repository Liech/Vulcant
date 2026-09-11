#pragma once

#include "Vulcant/Interface/VulcantGraphicCommand.h"
#include <memory>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;
    class VulcanPool;
    class VulcanShader;
    class VulcanSet;
    class VulcanGraphicCommand;
}

namespace Vulcant::VulcantG
{
    class VulcantGGraphicCommand : public VulcantGraphicCommand
    {
      public:
        VulcantGGraphicCommand(Vulcant::VulcantG::Wrapper::VulcanDevice& device);
        virtual ~VulcantGGraphicCommand();

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

        std::unique_ptr<Vulcant::VulcantG::Wrapper::VulcanGraphicCommand> cmd = nullptr;
    };
}
