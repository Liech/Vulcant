#include "GraphicPipelineTriangle.h"

#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantInput.h"
#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/VulcantV/VulcantVBuffer.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "Vulcant/VulcantV/VulcantVImage.h"
#include "Vulcant/Interface/VulcantGraphicCommand.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "Vulcant/VulcantV/VulcantVGraphicPipeline.h"
#include "Vulcant/Wrapper/Window.h"
#include "ShaderLibrary/Example/Triangle_vert.h"
#include "ShaderLibrary/Example/Triangle_frag.h"

namespace Vulcant::Examples
{
    void GraphicPipelineTriangle::demo()
    {
        auto windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();

        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);

        {
            Vulcant::Examples::GraphicPipelineTriangle example;
            example.createWindow(device, glm::ivec2(800, 600));

            auto& window = example.getWindow();

            while (!window.isClosed())
            {
                window.tick();
            }
        }
    }

    GraphicPipelineTriangle::GraphicPipelineTriangle() {}
    GraphicPipelineTriangle::~GraphicPipelineTriangle() {}

    Vulcant::VulcantWindow& GraphicPipelineTriangle::getWindow()
    {
        return *window;
    }

    void GraphicPipelineTriangle::createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        resolution = inputResolution;
        window     = deviceInput.createWindow(resolution, "Triangle!");

        prepare(deviceInput, resolution);

        auto rec = [this]()
        {
            auto vbuf = (Vulcant::VulcantV::VulcantVBuffer*)vertexBuffer.get();

            cmd->startRecord();
            cmd->beginRendering(*pipeline);
            cmd->setViewportAndScissor(glm::uvec2(resolution.x, resolution.y));
            cmd->draw(3, nullptr, vertexBuffer.get());
            cmd->endRendering();
            cmd->endRecord();
        };
        rec();

        window->start(
          [this](double) {
              // onLogic
          },
          [this]() {
              // onRender
              prepareRun();
              cmd->runAsync();
              window->blitImage(getResult());
          },
          [this, rec](const glm::ivec2& newResolution) {
              // onResize
              changeResolution(newResolution);
              rec();
          });

        window->getInput().setCallback(
          [this](const Vulcant::VulcantInputValue& key) {
          });
    }

    void GraphicPipelineTriangle::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        device     = &deviceInput;
        resolution = inputResolution;

        auto& vdevice = (Vulcant::VulcantV::VulcantVDevice&)deviceInput;

        depthImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);
        colorImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R8G8B8A8_UNORM);

        std::vector<float> vertices = {
            0.0f,  -0.5f, 1.0f, 0.0f, 0.0f, // vertex 0: pos (0.0, -0.5), col (1, 0, 0)
            0.5f,   0.5f, 0.0f, 1.0f, 0.0f, // vertex 1: pos (0.5, 0.5),  col (0, 1, 0)
           -0.5f,   0.5f, 0.0f, 0.0f, 1.0f  // vertex 2: pos (-0.5, 0.5), col (0, 0, 1)
        };
        vertexBuffer = device->createVertexBuffer(3, sizeof(float) * 5, false /*not gpu only*/);
        vertexBuffer->uploadToGPU(vertices.data(), 3, 0);

        cmd = deviceInput.createGraphicCommand();

        const auto* vert_slang = Triangle_vert_spirv;
        auto        vert_size  = Triangle_vert_spirv_sizeInBytes;
        auto        vert_spirv = std::vector<uint32_t>(vert_slang, vert_slang + vert_size / sizeof(uint32_t));

        const auto* frag_slang = Triangle_frag_spirv;
        auto        frag_size  = Triangle_frag_spirv_sizeInBytes;
        auto        frag_spirv = std::vector<uint32_t>(frag_slang, frag_slang + frag_size / sizeof(uint32_t));

        vertShader = deviceInput.createShader(vert_spirv);
        fragShader = deviceInput.createShader(frag_spirv);

        auto* dpt = depthImage.get();
        std::vector<Vulcant::VulcantShader*> shadersIf = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorIf   = { colorImage.get() };

        pipeline = deviceInput.createVulcanGraphicPipeline(shadersIf, colorIf, dpt, nullptr);
    }

    void GraphicPipelineTriangle::changeResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;
        depthImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);
        colorImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R8G8B8A8_UNORM);

        auto& vdevice = (Vulcant::VulcantV::VulcantVDevice&)*device;
        auto* dpt     = depthImage.get();

        std::vector<Vulcant::VulcantShader*> shadersIf = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorIf   = { colorImage.get() };

        pipeline = vdevice.createVulcanGraphicPipeline(shadersIf, colorIf, dpt, nullptr);
    }

    void GraphicPipelineTriangle::record(Vulcant::VulcantComputeCommand& cmd) {}

    void GraphicPipelineTriangle::prepareRun() {}

    Vulcant::VulcantImage& GraphicPipelineTriangle::getResult()
    {
        return *colorImage;
    }
}