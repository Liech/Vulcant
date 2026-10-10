#pragma once

#include "Example.h"
#include <memory>

namespace Vulcant
{
    class VulcantGraphicCommand;
    class VulcantUi;
}

namespace Vulcant::Rendering
{
    class MeshRenderer;
    class ArcCam;
}

namespace Vulcant::Examples
{
    class MeshRendererUi : public Example
    {
      public:
        static void demo();

        MeshRendererUi();
        virtual ~MeshRendererUi();

        std::string getName() override
        {
            return "MeshRenderer + ImGui";
        }

        std::string getDescription() override
        {
            return "MeshRenderer with ImGui overlay (Workbench scenario)";
        }

        void                    createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        void                    prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        void                    changeResolution(const glm::ivec2& newResolution) override;
        void                    record(Vulcant::VulcantComputeCommand& cmd) override;
        void                    prepareRun() override;
        Vulcant::VulcantImage&  getResult() override;
        Vulcant::VulcantWindow& getWindow() override;

      private:
        Vulcant::VulcantDevice*                           device = nullptr;
        std::unique_ptr<Vulcant::VulcantWindow>           window;
        glm::ivec2                                        resolution;
        std::unique_ptr<Vulcant::Rendering::MeshRenderer> meshRenderer;
        std::unique_ptr<Vulcant::Rendering::ArcCam>       cam;
        std::unique_ptr<Vulcant::VulcantUi>               ui;
        std::unique_ptr<Vulcant::VulcantGraphicCommand>   drawCmd;
        std::unique_ptr<Vulcant::VulcantGraphicCommand>   uiCmd;
    };
}
