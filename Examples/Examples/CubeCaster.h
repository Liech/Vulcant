#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Example.h"
#include "Library/Vulcant/Rendering/Light.h"
#include "Library/Vulcant/Rendering/SceneData.h"

namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantSet;
    class VulcantBuffer;
    class VulcantWindow;
    class VulcantComputeCommand;
}

namespace Vulcant::Rendering
{
    class DeferredShading;
}

namespace Vulcant::Examples
{
    class CubeCaster : public Example
    {
      public:
        static void demo();

        CubeCaster();
        virtual ~CubeCaster();

        virtual std::string getName() override
        {
            return "Cube Caster";
        }

        virtual std::string getDescription() override
        {
            return "3: Use of Defered Shading and pipeline chains. Demonstrate input. Raycast of multiple light lit cube.";
        };

        void setDevice(Vulcant::VulcantDevice& device, Vulcant::VulcantComputeCommand& cmd);
        void setResolution(const glm::ivec2& resolution);
        void setSceneData(const Vulcant::Rendering::SceneData&);
        void render();

        Vulcant::VulcantImage& getColor();
        Vulcant::VulcantImage& getDepth();
        Vulcant::VulcantImage& getNormal();

        virtual void                    createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                    prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
        virtual void                    changeResolution(const glm::ivec2& newResolution) override;
        virtual void                    record(Vulcant::VulcantComputeCommand& cmd) override;
        virtual void                    prepareRun() override;
        virtual Vulcant::VulcantImage&  getResult() override;
        virtual Vulcant::VulcantWindow& getWindow() override;

        Vulcant::Rendering::SceneData          sceneData;
        std::vector<Vulcant::Rendering::Light> lightData;

        double pos = 3.0;

      private:
        Vulcant::Rendering::Light     getExampleLight();
        Vulcant::Rendering::SceneData getScene(const glm::vec3& cameraPos, const glm::dvec2& mouse);

        glm::ivec2 resolution        = glm::ivec2(-1, -1);
        bool       sceneDataUploaded = false;

        Vulcant::VulcantDevice* device = nullptr;

        double                                               factor = 1.0;
        float                                                time   = 0.0f;
        glm::dvec2                                           mouse  = glm::dvec2(0, 0);
        std::unique_ptr<Vulcant::Rendering::DeferredShading> deferred;
        std::unique_ptr<Vulcant::VulcantBuffer>              sceneDataUbo;
        std::unique_ptr<Vulcant::VulcantShader>              shader;
        std::unique_ptr<Vulcant::VulcantSet>                 set;
        std::unique_ptr<Vulcant::VulcantImage>               color;
        std::unique_ptr<Vulcant::VulcantImage>               depth;
        std::unique_ptr<Vulcant::VulcantImage>               normal;
        std::unique_ptr<Vulcant::VulcantImage>               result;
        std::unique_ptr<Vulcant::VulcantComputeCommand>             cmd;
        std::unique_ptr<Vulcant::VulcantComputeCommand>             defcmd;
        std::unique_ptr<Vulcant::VulcantWindow>              window;
    };
}
