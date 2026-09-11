#pragma once

#include "Example.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantSet;
    class VulcantComputeCommand;
    class VulcantBuffer;
    class VulcantWindow;

}

namespace Vulcant::Examples
{
    struct PartialSceneData
    {
        glm::mat4 viewMatrix;
        glm::mat4 inv_projection;
        glm::vec4 lightPos; // Using vec4 for padding/alignment
    };

    class SphereImage : public Example
    {
      public:
        SphereImage();
        virtual ~SphereImage();

        static void demo();

        virtual std::string getName() override
        {
            return "Sphere Image";
        }

        virtual std::string getDescription() override
        {
            return "2: Use of a Buffer and Image. Small static sphere raycast. Change between glsl and slang";
        };
        bool useSlang = false;

        virtual void                    createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                   prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                   changeResolution(const glm::ivec2& newResolution) override;
        virtual void                   record(Vulcant::VulcantComputeCommand& cmd) override;
        virtual void                   prepareRun() override;
        virtual Vulcant::VulcantImage& getResult() override;
        virtual Vulcant::VulcantWindow& getWindow() override;

      private:
        std::string             getSource();
        Vulcant::VulcantDevice* device;

        PartialSceneData scene;
        glm::ivec2       resolution = glm::ivec2(1920, 1080);

        std::unique_ptr<Vulcant::VulcantImage>   img;
        std::unique_ptr<Vulcant::VulcantShader>  shader;
        std::unique_ptr<Vulcant::VulcantSet>     set;
        std::unique_ptr<Vulcant::VulcantBuffer>  ubo;
        std::unique_ptr<Vulcant::VulcantComputeCommand> cmd;
        std::unique_ptr<Vulcant::VulcantWindow>  window;
    };
}