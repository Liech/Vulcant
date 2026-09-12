#include "VulcantGGraphicCommand.h"

#include "VulcantG/Wrapper/VulcanGraphicCommand.h"
#include "VulcantG/Wrapper/VulcanBuffer.h"
#include "VulcantG/Wrapper/VulcanImage.h"
#include "VulcantG/Wrapper/VulcanSet.h"
#include "VulcantG/VulcantG/VulcantGBuffer.h"
#include "VulcantG/VulcantG/VulcantGGraphicPipeline.h"
#include "VulcantG/VulcantG/VulcantGImage.h"
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
        cmd->setViewportAndScissor(extent);
    }

    void VulcantGGraphicCommand::draw(uint32_t vertexCount, VulcantSet* set, VulcantBuffer* vertexBuffer) {
        VulcantG::Wrapper::VulcanSet* gSet = nullptr;
        if (set)
            gSet = ((VulcantGSet*)set)->set.get();

        VulcantG::Wrapper::VulcanBuffer* gBuf = nullptr;
        if (vertexBuffer)
            gBuf = static_cast<VulcantGBuffer*>(vertexBuffer)->buffer.get();

        cmd->draw(vertexCount, gSet, gBuf);
    }

    void VulcantGGraphicCommand::addBarrier(VulcantImage& inputImg, const VulcantResourceLayout& dest) {
        cmd->addBarrier(*static_cast<VulcantGImage&>(inputImg).img, dest);
    }
    void VulcantGGraphicCommand::addBarrier(VulcantBuffer& buffer) {
        cmd->addBarrier(*static_cast<VulcantGBuffer&>(buffer).buffer);
    }

    void VulcantGGraphicCommand::runAsync() {
        cmd->runAsync();
    }
    void VulcantGGraphicCommand::runSync() {
        cmd->runSync();
    }
    void VulcantGGraphicCommand::wait() {
        cmd->wait();
    }
}
