#pragma once

#include "Example.h"
#include <memory>
#include <string>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantGraphicCommand;
    class VulcantSet;
    class VulcantComputeCommand;
    class VulcantWindow;
    class VulcantBuffer;
    class VulcantGraphicPipeline;
}

namespace Vulcant::Examples
{
    class GraphicPipelineTriangle : public Example
    {
      public:
        static void demo();

        GraphicPipelineTriangle();
        virtual ~GraphicPipelineTriangle();

        virtual std::string getName() override
        {
            return "Graphic Pipeline Triangle";
        }

        virtual std::string getDescription() override
        {
            return "1: Minimal graphic pipeline example. A triangle rendered with a simple vertex and fragment shader.";
        };
        bool useSlang = false;

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
        std::unique_ptr<Vulcant::VulcantShader>          vertShader;
        std::unique_ptr<Vulcant::VulcantShader>          fragShader;
        std::unique_ptr<Vulcant::VulcantGraphicCommand>  cmd;
        std::unique_ptr<Vulcant::VulcantGraphicPipeline> pipeline;
    };
}