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
    class BlitDepthAwareShader
    {
      public:
        BlitDepthAwareShader();
        virtual ~BlitDepthAwareShader();

        void setDevice(Vulcant::VulcantDevice& device, Vulcant::VulcantComputeCommand& cmd);
        void setInputTextures(Vulcant::VulcantImage&                    input,
                              Vulcant::VulcantImage&                    input_depth,
                              std::shared_ptr<Vulcant::VulcantResource> screen,
                              std::shared_ptr<Vulcant::VulcantResource> screen_depth);
        void render();

      private:
        Vulcant::VulcantImage* input       = nullptr;
        Vulcant::VulcantImage* input_depth = nullptr;

        Vulcant::VulcantDevice*  device = nullptr;
        Vulcant::VulcantComputeCommand* cmd    = nullptr;

        std::unique_ptr<Vulcant::VulcantShader> shader;
        std::unique_ptr<Vulcant::VulcantSet>    set;
    };
}