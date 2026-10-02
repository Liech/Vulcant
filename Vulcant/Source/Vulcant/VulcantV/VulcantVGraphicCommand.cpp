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

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>
#include "VulcantVDevice.h"

TEST_CASE("VulcantVGraphicCommand Setup and Pipeline Recording", "[VulcantVGraphicCommand]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    std::string vertShaderSource = R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        }
    )";

    std::string fragShaderSource = R"(
        #version 450
        layout(location = 0) out vec4 outColor;
        void main() {
            outColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    auto vertShader = device.createShader(vertShaderSource);
    auto fragShader = device.createShader(fragShaderSource);
    REQUIRE(vertShader != nullptr);
    REQUIRE(fragShader != nullptr);

    auto colorImage = device.createImage(800, 600, 1, VulcantImageFormat::R8G8B8A8_UNORM);
    REQUIRE(colorImage != nullptr);

    std::vector<Vulcant::VulcantShader*> shaders = { vertShader.get(), fragShader.get() };
    std::vector<Vulcant::VulcantImage*> colorAttachments = { colorImage.get() };

    auto pipeline = device.createVulcanGraphicPipeline(shaders, colorAttachments, nullptr, nullptr);
    REQUIRE(pipeline != nullptr);

    auto graphicCmd = device.createGraphicCommand();
    REQUIRE(graphicCmd != nullptr);

    graphicCmd->startRecord();
    graphicCmd->addBarrier(*colorImage, VulcantResourceLayout::ColorAttachment);
    graphicCmd->beginRendering(*pipeline);
    graphicCmd->setViewportAndScissor(glm::uvec2(800, 600));
    graphicCmd->draw(3, nullptr, nullptr, 1);
    graphicCmd->endRendering();
    graphicCmd->endRecord();

    graphicCmd->runSync();
}
#endif
