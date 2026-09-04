#pragma once

#include "Example.h"
#include <memory>
#include <string>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantSet;
    class VulcantComputeCommand;
    class VulcantWindow;
}

namespace Vulcant::Examples
{
    class MandelbrotImage : public Example
    {
      public:
        static void demo();

        MandelbrotImage();
        virtual ~MandelbrotImage();

        virtual std::string getName() override
        {
            return "Mangelbrot Image";
        }

        virtual std::string getDescription() override
        {
            return "1: Minimal image example. An image of the mandelbrot Fractal. Change between glsl and slang";
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
        static std::string      getSource();
        static inline const int WORKGROUP_SIZE = 32;

        Vulcant::VulcantDevice* device;
        glm::ivec2              resolution = glm::ivec2(1920, 1080);

        std::unique_ptr<Vulcant::VulcantImage>   img;
        std::unique_ptr<Vulcant::VulcantShader>  shader;
        std::unique_ptr<Vulcant::VulcantSet>     set;
        std::unique_ptr<Vulcant::VulcantComputeCommand> cmd;
        std::unique_ptr<Vulcant::VulcantWindow>  window;
    };
}