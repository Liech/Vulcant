#include "VulcantGGraphicCommand.h"

#include "VulcantG/Wrapper/VulcanGraphicCommand.h"
#include "VulcantG/VulcantG/VulcantGGraphicPipeline.h"
#include "VulcantG/VulcantG/VulcantGSet.h"

namespace Vulcant::VulcantG
{
    VulcantGGraphicCommand::VulcantGGraphicCommand(Vulcant::VulcantG::Wrapper::VulcanDevice& device) {
        cmd = std::make_unique<Vulcant::VulcantG::Wrapper::VulcanGraphicCommand>(device);
    }

    VulcantGGraphicCommand::~VulcantGGraphicCommand() {}

    void VulcantGGraphicCommand::startRecord() {
        cmd->startRecord();
    }
    void VulcantGGraphicCommand::endRecord() {
        cmd->endRecord();
    }

    void VulcantGGraphicCommand::beginRendering(Vulcant::VulcantGraphicPipeline& pipeline) {
        cmd->beginRendering(*((VulcantGGraphicPipeline&)pipeline).pipe.get());
    }
    void VulcantGGraphicCommand::endRendering() {
        cmd->endRendering();
    }

    void VulcantGGraphicCommand::setViewportAndScissor(glm::uvec2 extent) {
        //cmd->setViewportAndScissor(extent);
    }

    void VulcantGGraphicCommand::draw(uint32_t vertexCount, VulcantSet* set, VulcantBuffer* vertexBuffer) {
        //VulcantG::Wrapper::VulcanSet* gSet = nullptr;
        //if (set)
        //    gSet = ((VulcantGSet*)set)->set.get();
        //
        //cmd->draw(vertexCount,gSet , vertexBuffer);
    }

    void VulcantGGraphicCommand::addBarrier(VulcantImage& inputImg, const VulcantResourceLayout& dest) {}
    void VulcantGGraphicCommand::addBarrier(VulcantBuffer& buffer) {}

    void VulcantGGraphicCommand::runAsync() {}
    void VulcantGGraphicCommand::runSync() {}
    void VulcantGGraphicCommand::wait() {}
}
