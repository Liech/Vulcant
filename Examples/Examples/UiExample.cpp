#include "UiExample.h"

#include "Library/Vulcant/Interface/VulcantDevice.h"
#include "Library/Vulcant/Interface/VulcantGraphicCommand.h"
#include "Library/Vulcant/Interface/VulcantImage.h"
#include "Library/Vulcant/Interface/VulcantUi.h"
#include "Library/Vulcant/Interface/VulcantWindow.h"
#include "Library/Vulcant/VulcantV/VulcantVDevice.h"
#include "Library/Vulcant/Wrapper/Window.h"

#include <imgui.h>

namespace Vulcant::Examples
{
    void UiExample::demo()
    {
        auto                              windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();
        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);
        Vulcant::Examples::UiExample      example;
        example.createWindow(device, glm::ivec2(1280, 720));
        auto& window = example.getWindow();

        while (!window.isClosed())
        {
            window.tick();
        }
    }

    UiExample::UiExample() {}
    UiExample::~UiExample() {}

    Vulcant::VulcantWindow& UiExample::getWindow()
    {
        return *window;
    }

    void UiExample::createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        resolution = inputResolution;
        window     = deviceInput.createWindow(resolution, "Vulcant + Dear ImGui Example");

        prepare(deviceInput, resolution);

        window->start(
          [this](double /*dt*/)
          {
              // onLogic: build Dear ImGui frames and widgets
              ui->newFrame();

              ImGui::Begin("Vulcant Control Panel");
              ImGui::Text("Application average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
              ImGui::Text("Resolution: %d x %d", resolution.x, resolution.y);
              ImGui::Separator();

              if (ImGui::Button("Click Me!"))
              {
                  counter++;
              }
              ImGui::SameLine();
              ImGui::Text("Button clicks: %d", counter);

              ImGui::SliderFloat("Slider Demo", &sliderValue, 0.0f, 1.0f);
              ImGui::InputText("Text Input", textBuffer, sizeof(textBuffer));

              ImGui::Checkbox("Show ImGui Demo Window", &showDemo);
              ImGui::End();

              if (showDemo)
              {
                  ImGui::ShowDemoWindow(&showDemo);
              }
          },
          [this]()
          {
              // onRender: record UI and display on window
              cmd->startRecord();
              ui->record(*cmd, *img);
              cmd->endRecord();

              cmd->runAsync();
              window->blitImage(*img);
          },
          [this](const glm::ivec2& newResolution)
          {
              // onResize
              changeResolution(newResolution);
          });
    }

    void UiExample::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        device     = &deviceInput;
        resolution = inputResolution;

        ui  = device->createUi(*window);
        cmd = device->createGraphicCommand();

        changeResolution(resolution);
    }

    void UiExample::changeResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;
        img        = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R8G8B8A8_UNORM);
    }

    void UiExample::record(Vulcant::VulcantComputeCommand& /*cmd*/) {}

    void UiExample::prepareRun() {}

    Vulcant::VulcantImage& UiExample::getResult()
    {
        return *img;
    }
}