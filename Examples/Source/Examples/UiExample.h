#pragma once

#include "Example.h"
#include <memory>
#include <string>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantGraphicCommand;
    class VulcantComputeCommand;
    class VulcantWindow;
    class VulcantUi;
}

namespace Vulcant::Examples
{
    class UiExample : public Example
    {
      public:
        static void demo();

        UiExample();
        virtual ~UiExample();

        virtual std::string getName() override
        {
            return "Ui Example";
        }

        virtual std::string getDescription() override
        {
            return "5: Dear ImGui UI interaction example.";
        };

        virtual void                    createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                    prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                    changeResolution(const glm::ivec2& newResolution) override;
        virtual void                    record(Vulcant::VulcantComputeCommand& cmd) override;
        virtual void                    prepareRun() override;
        virtual Vulcant::VulcantImage&  getResult() override;
        virtual Vulcant::VulcantWindow& getWindow() override;

      private:
        Vulcant::VulcantDevice* device     = nullptr;
        glm::ivec2              resolution = glm::ivec2(1280, 720);

        std::unique_ptr<Vulcant::VulcantWindow>         window;
        std::unique_ptr<Vulcant::VulcantUi>             ui;
        std::unique_ptr<Vulcant::VulcantGraphicCommand> cmd;
        std::unique_ptr<Vulcant::VulcantImage>          img;

        int   counter         = 0;
        float sliderValue     = 0.5f;
        bool  showDemo        = true;
        char  textBuffer[128] = "Hello Vulcant + ImGui!";
    };
}