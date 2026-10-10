#include "MeshRendererUi.h"

#include "Rendering/ArcCam.h"
#include "Rendering/MeshRenderer.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantGraphicCommand.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantInput.h"
#include "Vulcant/Interface/VulcantUi.h"
#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "Vulcant/Wrapper/Window.h"

#include <imgui.h>

namespace Vulcant::Examples
{
    void MeshRendererUi::demo()
    {
        auto                              windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();
        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);
        Vulcant::Examples::MeshRendererUi example;
        example.createWindow(device, glm::ivec2(1280, 720));
        auto& window = example.getWindow();

        while (!window.isClosed())
        {
            window.tick();
        }
    }

    MeshRendererUi::MeshRendererUi() {}
    MeshRendererUi::~MeshRendererUi() {}

    Vulcant::VulcantWindow& MeshRendererUi::getWindow()
    {
        return *window;
    }

    Vulcant::VulcantImage& MeshRendererUi::getResult()
    {
        return meshRenderer->getColor();
    }

    void MeshRendererUi::createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        resolution = inputResolution;
        window     = deviceInput.createWindow(resolution, "MeshRenderer + ImGui Reproduction");

        prepare(deviceInput, resolution);

        auto rec = [this]()
        {
            if (drawCmd)
                drawCmd->wait();
            if (uiCmd)
                uiCmd->wait();

            drawCmd = device->createGraphicCommand();
            uiCmd   = device->createGraphicCommand();

            drawCmd->startRecord();
            if (meshRenderer)
            {
                meshRenderer->record(*drawCmd);
            }
            drawCmd->endRecord();
        };

        rec();

        window->start(
          [this](double delta)
          {
              if (cam)
              {
                  cam->tick(delta);
              }

              ui->newFrame();

              ImGui::Begin("MeshRenderer + ImGui (Bug Reproduction)");
              ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / (ImGui::GetIO().Framerate > 0 ? ImGui::GetIO().Framerate : 1.0f));
              ImGui::Separator();
              ImGui::Text("This minimal example reproduces the Workbench setup:");
              ImGui::BulletText("MeshRenderer renders on R32G32B32A32_SFLOAT");
              ImGui::BulletText("ImGui renders on top with clear = false");
              ImGui::Separator();
              if (ImGui::Button("Click Me"))
              {
              }
              static float slider = 0.5f;
              ImGui::SliderFloat("Slider Demo", &slider, 0.0f, 1.0f);
              ImGui::End();
          },
          [this]()
          {
              prepareRun();

              drawCmd->runAsync();
              drawCmd->wait();

              uiCmd->startRecord();
              ui->record(*uiCmd, getResult());
              uiCmd->endRecord();
              uiCmd->runAsync();
              uiCmd->wait();

              window->blitImage(getResult());
          },
          [this, rec](const glm::ivec2& newResolution)
          {
              changeResolution(newResolution);
              rec();
          });

        window->getInput().setCallback(
          [this](const Vulcant::VulcantInputValue& key)
          {
              if (cam && cam->keyEvent(key))
                  return;
          });
    }

    void MeshRendererUi::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        device     = &deviceInput;
        resolution = inputResolution;

        meshRenderer = std::make_unique<Vulcant::Rendering::MeshRenderer>(*device, resolution);
        cam          = std::make_unique<Vulcant::Rendering::ArcCam>(*window, glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

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

        std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0, // back
            4, 5, 6, 6, 7, 4, // front
            0, 4, 7, 7, 3, 0, // left
            1, 5, 6, 6, 2, 1, // right
            3, 2, 6, 6, 7, 3, // top
            0, 1, 5, 5, 4, 0  // bottom
        };

        meshRenderer->setMesh(positions, indices, glm::vec3(0.3f, 0.6f, 0.9f));

        ui = device->createUi(*window, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT, false);
    }

    void MeshRendererUi::changeResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;
        if (meshRenderer)
        {
            meshRenderer->setResolution(resolution);
        }
    }

    void MeshRendererUi::record(Vulcant::VulcantComputeCommand& /*cmd*/) {}

    void MeshRendererUi::prepareRun()
    {
        if (cam && meshRenderer)
        {
            meshRenderer->setSceneData(cam->getScene());
        }
    }
}
