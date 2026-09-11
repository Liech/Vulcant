#include "GraphicPipelineCube.h"
#include "Vulcant/Interface/VulcantShader.h"

#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantGraphicCommand.h"
#include "Vulcant/Interface/VulcantGraphicPipeline.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantInput.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/VulcantV/VulcantVBuffer.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "Vulcant/VulcantV/VulcantVGraphicPipeline.h"
#include "Vulcant/VulcantV/VulcantVImage.h"
#include "Vulcant/Wrapper/Window.h"
#include "ShaderLibrary/Example/Cube_frag.h"
#include "ShaderLibrary/Example/Cube_vert.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace Vulcant::Examples
{
    void GraphicPipelineCube::demo()
    {
        auto windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();

        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);

        {
            GraphicPipelineCube example;
            example.createWindow(device, glm::ivec2(800, 600));

            auto& window = example.getWindow();

            while (!window.isClosed())
            {
                window.tick();
            }
        }
    }

    GraphicPipelineCube::GraphicPipelineCube() {}
    GraphicPipelineCube::~GraphicPipelineCube() {}

    Vulcant::VulcantWindow& GraphicPipelineCube::getWindow()
    {
        return *window;
    }

    void GraphicPipelineCube::createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        resolution = inputResolution;
        window     = deviceInput.createWindow(resolution, "Cube!");

        prepare(deviceInput, resolution);

        auto rec = [this]()
        {
            cmd->startRecord();
            cmd->beginRendering(*pipeline);
            cmd->setViewportAndScissor(glm::uvec2(resolution.x, resolution.y));
            // bind descriptor set containing transform UBO
            cmd->draw(36, set.get(), vertexBuffer.get());
            cmd->endRendering();
            cmd->endRecord();
        };
        rec();

        window->start(
          [this](double dt)
          {
              // onLogic: update rotation angle only
              angle += float(dt) * 0.8f;
          },
          [this]()
          {
              // onRender
              prepareRun();
              cmd->runAsync();
              window->blitImage(getResult());
          },
          [this, rec](const glm::ivec2& newResolution)
          {
              // onResize
              changeResolution(newResolution);
              rec();
          });

        window->getInput().setCallback([this](const Vulcant::VulcantInputValue& key) {});
    }

    void GraphicPipelineCube::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        device     = &deviceInput;
        resolution = inputResolution;

        auto& vdevice = (Vulcant::VulcantV::VulcantVDevice&)deviceInput;

        depthImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);
        colorImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R8G8B8A8_UNORM);

        // Create cube vertex buffer (36 vertices, each pos.xyz + color.rgb)
        std::vector<float> vertices;
        vertices.reserve(36 * 6);

        // cube corners
        std::vector<glm::vec3> positions = {
            { -0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f, -0.5f },
            {  0.5f,  0.5f, -0.5f },
            { -0.5f,  0.5f, -0.5f },
            { -0.5f, -0.5f,  0.5f },
            {  0.5f, -0.5f,  0.5f },
            {  0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f }
        };

        // indices for 12 triangles
        const uint32_t idx[] = {
            0, 1, 2, 2, 3, 0, // back
            4, 5, 6, 6, 7, 4, // front
            0, 4, 7, 7, 3, 0, // left
            1, 5, 6, 6, 2, 1, // right
            3, 2, 6, 6, 7, 3, // top
            0, 1, 5, 5, 4, 0  // bottom
        };

        // color per face
        std::vector<glm::vec3> faceColors = {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
            { 1, 1, 0 },
            { 1, 0, 1 },
            { 0, 1, 1 }
        };

        for (int f = 0; f < 6; f++)
        {
            glm::vec3 color = faceColors[f];
            for (int v = 0; v < 6; v++)
            {
                uint32_t vi = idx[f * 6 + v];
                auto     p  = positions[vi];

                vertices.push_back(p.x);
                vertices.push_back(p.y);
                vertices.push_back(p.z);
                vertices.push_back(color.r);
                vertices.push_back(color.g);
                vertices.push_back(color.b);
            }
        }

        vertexBuffer = device->createVertexBuffer(36, sizeof(float) * 6, false);
        vertexBuffer->uploadToGPU(vertices.data(), 36, 0);

        transformUbo = device->createUniform(1, sizeof(glm::mat4));

        cmd = deviceInput.createGraphicCommand();

        const auto* vert_slang = Cube_vert_spirv;
        auto        vert_size  = Cube_vert_spirv_sizeInBytes;
        auto        vert_spirv = std::vector<uint32_t>(vert_slang, vert_slang + vert_size / sizeof(uint32_t));

        const auto* frag_slang = Cube_frag_spirv;
        auto        frag_size  = Cube_frag_spirv_sizeInBytes;
        auto        frag_spirv = std::vector<uint32_t>(frag_slang, frag_slang + frag_size / sizeof(uint32_t));

        vertShader = deviceInput.createShader(vert_spirv);
        fragShader = deviceInput.createShader(frag_spirv);

        auto*                                dpt       = depthImage.get();
        std::vector<Vulcant::VulcantShader*> shadersIf = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorIf   = { colorImage.get() };

        pipeline = deviceInput.createVulcanGraphicPipeline(shadersIf, colorIf, dpt, nullptr);
        set = device->createSet({ { transformUbo->asResource() } }, *vertShader);
    }

    void GraphicPipelineCube::changeResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;
        depthImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);
        colorImage = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R8G8B8A8_UNORM);

        auto& vdevice = (Vulcant::VulcantV::VulcantVDevice&)*device;
        auto* dpt     = depthImage.get();

        std::vector<Vulcant::VulcantShader*> shadersIf = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorIf   = { colorImage.get() };

        pipeline = vdevice.createVulcanGraphicPipeline(shadersIf, colorIf, dpt, nullptr);
        set = device->createSet({ { transformUbo->asResource() } }, *vertShader);
    }

    void GraphicPipelineCube::record(Vulcant::VulcantComputeCommand& cmd) {}
    void GraphicPipelineCube::prepareRun()
    {
        float     aspect = float(resolution.x) / float(resolution.y);
        glm::mat4 proj   = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
        proj[1][1] *= -1;

        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model           = glm::rotate(model, angle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 mvp = proj * view * model;

        transformUbo->uploadToGPU(glm::value_ptr(mvp), 1, 0);
    }

    Vulcant::VulcantImage& GraphicPipelineCube::getResult()
    {
        return *colorImage;
    }
}
