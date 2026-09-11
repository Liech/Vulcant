#pragma once

#include "Example.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"
#include <memory>
#include <string>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantGraphicCommand;
    class VulcantComputeCommand;
    class VulcantWindow;
    class VulcantBuffer;
    class VulcantSet;
    class VulcantGraphicPipeline;
}

namespace Vulcant::Wrapper
{
    class VulcanGraphicPipeline;
}

namespace Vulcant::Examples
{
    class GraphicPipelineCube : public Example
    {
      public:
        static void demo();

        GraphicPipelineCube();
        virtual ~GraphicPipelineCube();

        virtual std::string getName() override
        {
            return "Graphic Pipeline Cube";
        }

        virtual std::string getDescription() override
        {
            return "Graphic pipeline example rendering a colored cube (CPU transform).";
        };

        virtual void                    createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                    prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                    changeResolution(const glm::ivec2& newResolution) override;
        virtual void                    record(Vulcant::VulcantComputeCommand& cmd) override;
        virtual void                    prepareRun() override;
        virtual Vulcant::VulcantImage&  getResult() override;
        virtual Vulcant::VulcantWindow& getWindow() override;

      private:
        std::unique_ptr<Vulcant::VulcantWindow> window;

        Vulcant::VulcantDevice* device     = nullptr;
        glm::ivec2              resolution = glm::ivec2(-1, -1);

        std::unique_ptr<Vulcant::VulcantImage>           depthImage;
        std::unique_ptr<Vulcant::VulcantImage>           colorImage;
        std::unique_ptr<Vulcant::VulcantBuffer>          vertexBuffer;
        std::unique_ptr<Vulcant::VulcantBuffer>          transformUbo;
        std::unique_ptr<Vulcant::VulcantSet>             set;
        std::unique_ptr<Vulcant::VulcantShader>          vertShader;
        std::unique_ptr<Vulcant::VulcantShader>          fragShader;
        std::unique_ptr<Vulcant::VulcantGraphicCommand>  cmd;
        std::unique_ptr<Vulcant::VulcantGraphicPipeline> pipeline;
        float                                            angle = 0.0f;
    };
}
