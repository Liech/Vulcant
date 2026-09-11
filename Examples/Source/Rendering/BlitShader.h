#pragma once

#include "Light.h"
#include "SceneData.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantSet;
    class VulcantComputeCommand;
    class VulcantBuffer;
    class VulcantResource;
}

namespace Vulcant::Rendering
{
    class BlitShader
    {
      public:
        BlitShader();
        virtual ~BlitShader();

        void setDevice(Vulcant::VulcantDevice& device);
        void setInputTextures(Vulcant::VulcantImage& input, std::shared_ptr<Vulcant::VulcantResource> screen);
        void render();

      private:
        Vulcant::VulcantImage* input = nullptr;

        Vulcant::VulcantDevice*                  device = nullptr;
        std::unique_ptr<Vulcant::VulcantComputeCommand> cmd    = nullptr;

        std::unique_ptr<Vulcant::VulcantShader> shader;
        std::unique_ptr<Vulcant::VulcantSet>    set;
    };
}