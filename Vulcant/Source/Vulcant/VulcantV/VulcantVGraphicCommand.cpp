#include "VulcantVGraphicCommand.h"

#include "Vulcant/Wrapper/Graphic/VulcanGraphicCommand.h"
#include "Vulcant/Wrapper/VulcanImage.h"
#include "Vulcant/Wrapper/VulcanShader.h"
#include "Vulcant/VulcantV/VulcantVImage.h"
#include "Vulcant/VulcantV/VulcantVShader.h"
#include "Vulcant/VulcantV/VulcantVSet.h"
#include "Vulcant/VulcantV/VulcantVGraphicPipeline.h"
#include "Vulcant/VulcantV/VulcantVBuffer.h"

namespace Vulcant::VulcantV
{
    VulcantVGraphicCommand::VulcantVGraphicCommand(Wrapper::VulcanPool& pool, Wrapper::VulcanDevice& device)
    {
        cmd = std::make_unique<Wrapper::VulcanGraphicCommand>(pool, device);
    }

    VulcantVGraphicCommand::~VulcantVGraphicCommand() {}

    void VulcantVGraphicCommand::runSync()
    {
        cmd->runSync();
    }
    void VulcantVGraphicCommand::runAsync()
    {
        cmd->runAsync();
    }
    void VulcantVGraphicCommand::wait()
    {
        cmd->wait();
    }

    void VulcantVGraphicCommand::startRecord()
    {
        cmd->startRecord();
    }
    void VulcantVGraphicCommand::endRecord()
    {
        cmd->endRecord();
    }

    void VulcantVGraphicCommand::beginRendering(Vulcant::VulcantGraphicPipeline& pipeline)
    {
        auto& vPipe = (Vulcant::VulcantV::VulcantVGraphicPipeline&)pipeline;
        cmd->beginRendering(*vPipe.pipe);
    }

    void VulcantVGraphicCommand::endRendering()
    {
        cmd->endRendering();
    }

    void VulcantVGraphicCommand::setViewportAndScissor(glm::uvec2 extent)
    {
        cmd->setViewportAndScissor(extent);
    }

    void VulcantVGraphicCommand::draw(uint32_t vertexCount, VulcantSet* set, VulcantBuffer* vertexBuffer, uint32_t instanceCount)
    {
        Wrapper::VulcanSet* s = nullptr;
        if (set)
            s = ((VulcantVSet*)set)->set.get();

        Wrapper::VulcanBuffer* vb = nullptr;
        if (vertexBuffer)
        {
            vb = static_cast<VulcantVBuffer*>(vertexBuffer)->buffer.get();
        }

        cmd->draw(vertexCount, s, vb, instanceCount);
    }

    void VulcantVGraphicCommand::drawIndirect(VulcantBuffer& indirectBuffer, VulcantSet* set, VulcantBuffer* vertexBuffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
    {
        Wrapper::VulcanSet* s = nullptr;
        if (set)
            s = ((VulcantVSet*)set)->set.get();

        Wrapper::VulcanBuffer* vb = nullptr;
        if (vertexBuffer)
        {
            vb = static_cast<VulcantVBuffer*>(vertexBuffer)->buffer.get();
        }

        auto& ib = static_cast<VulcantVBuffer&>(indirectBuffer);
        cmd->drawIndirect(*ib.buffer, s, vb, offset, drawCount, stride);
    }

    void VulcantVGraphicCommand::addBarrier(VulcantImage& inputImg, const VulcantResourceLayout& dest)
    {
        cmd->addBarrier(*((VulcantVImage&)inputImg).img, dest);
    }

    void VulcantVGraphicCommand::addBarrier(VulcantBuffer& buffer)
    {
        cmd->addBarrier(*static_cast<VulcantVBuffer&>(buffer).buffer);
    }
}
