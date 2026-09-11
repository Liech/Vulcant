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
    class VulcantBuffer;
    class VulcantResource;
    class VulcantComputeCommand;
}

namespace Vulcant::Rendering
{
    class DeferredShading
    {
      public:
        DeferredShading();
        virtual ~DeferredShading();

        void prepare(Vulcant::VulcantDevice& device);
        void setSceneData(const SceneData&);
        void setLight(const std::vector<Light>&);
        void setInputTextures(Vulcant::VulcantImage& color, Vulcant::VulcantImage& depth, Vulcant::VulcantImage& normal);
        void record(Vulcant::VulcantComputeCommand& cmd);

        Vulcant::VulcantImage& getTexture() const;

      private:
        Vulcant::VulcantImage* color  = nullptr;
        Vulcant::VulcantImage* depth  = nullptr;
        Vulcant::VulcantImage* normal = nullptr;

        Vulcant::VulcantDevice*  device = nullptr;

        SceneData          sceneData;
        std::vector<Light> light;

        std::unique_ptr<Vulcant::VulcantShader> shader;
        std::unique_ptr<Vulcant::VulcantBuffer> sceneDataUbo;
        std::unique_ptr<Vulcant::VulcantBuffer> LightUbo;
        std::unique_ptr<Vulcant::VulcantSet>    set;
        std::unique_ptr<Vulcant::VulcantImage>  result;
    };
}