#include "CubeCasterFreecam.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <string_view>

#include "Library/Vulcant/Interface/VulcantBuffer.h"
#include "Library/Vulcant/Interface/VulcantComputeCommand.h"
#include "Library/Vulcant/Interface/VulcantDevice.h"
#include "Library/Vulcant/Interface/VulcantImage.h"
#include "Library/Vulcant/Interface/VulcantInput.h"
#include "Library/Vulcant/Interface/VulcantResource.h"
#include "Library/Vulcant/Interface/VulcantSet.h"
#include "Library/Vulcant/Interface/VulcantShader.h"
#include "Library/Vulcant/Interface/VulcantWindow.h"
#include "Library/Vulcant/Rendering/DeferredShading.h"
#include "Library/Vulcant/Rendering/Freecam.h"
#include "Library/Vulcant/Wrapper/Window.h"
#include "Library/Vulcant/VulcantV/VulcantVDevice.h"
#include "ShaderLibrary/Example/CubeCaster.h"

namespace Vulcant::Examples
{
    void CubeCasterFreecam::demo()
    {
        auto                                 windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();
        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);
        {
            Vulcant::Examples::CubeCasterFreecam img;
            img.createWindow(device, glm::ivec2(800, 600)); // full window render loop
            auto& window = img.getWindow();

            while (!window.isClosed())
            {
                window.tick();
            }
        }
    }

    CubeCasterFreecam::CubeCasterFreecam()
    {
        lightData = { getExampleLight() };
    }

    CubeCasterFreecam::~CubeCasterFreecam() {}

    Vulcant::VulcantWindow& CubeCasterFreecam::getWindow()
    {
        return *window;
    }

    void CubeCasterFreecam::createWindow(Vulcant::VulcantDevice& device, const glm::ivec2& res)
    {
        resolution = res;
        window     = device.createWindow(resolution, "Cube Window (WASD)");

        glm::vec3 eye    = glm::vec3(0.0, 0.0, 3.0);
        glm::vec3 center = glm::vec3(0,0, 0.0);
        glm::vec3 up     = glm::vec3(0.0, 1.0, 0.0);
        cam        = std::make_unique<Vulcant::Rendering::Freecam>(*window, eye,center,up);
        prepare(device, resolution);

        auto rec = [this, &device]()
        {
            cmd    = device.createComputeCommand();
            defcmd = device.createComputeCommand();
            cmd->startRecord();
            cmd->addBarrier(getColor(), Vulcant::VulcantResourceLayout::General);
            cmd->addBarrier(getDepth(), Vulcant::VulcantResourceLayout::General);
            cmd->addBarrier(getNormal(), Vulcant::VulcantResourceLayout::General);
            record(*cmd);
            cmd->addBarrier(getColor(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
            cmd->addBarrier(getDepth(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
            cmd->addBarrier(getNormal(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
            cmd->endRecord();

            defcmd->startRecord();
            defcmd->addBarrier(deferred->getTexture(), Vulcant::VulcantResourceLayout::General);
            deferred->record(*defcmd);
            defcmd->addBarrier(deferred->getTexture(), Vulcant::VulcantResourceLayout::TransferSrc);
            defcmd->endRecord();
        };
        rec();

        window->start([this](double delta) { // onLogic
              cam->tick(delta);
            },
          [this]()
          {//onRender
              prepareRun();
              cmd->runAsync();
              defcmd->runAsync();
              window->blitImage(getResult());
          },
          [this, rec](const glm::ivec2& newResolution)
          {//onResize
              changeResolution(newResolution);
              rec();
          });

        window->getInput().setCallback(
          [this](const Vulcant::VulcantInputValue& key)
          {
              if (cam->keyEvent(key))
                  return;
          });
    }

    void CubeCasterFreecam::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution)
    {
        device = &deviceInput;
        // scene
        sceneDataUbo = device->createUniform(1, sizeof(Vulcant::Rendering::SceneData));

        // shader
        const auto* slang      = CubeCaster_spirv;
        auto        slang_size = CubeCaster_spirv_sizeInBytes;
        auto        spirv      = std::vector<uint32_t>(slang, slang + slang_size / sizeof(uint32_t));

        shader = device->createShader(spirv);

        // deferred
        deferred = std::make_unique<Vulcant::Rendering::DeferredShading>();
        deferred->prepare(*device);

        // set & images
        changeResolution(resolution);
    }

    void CubeCasterFreecam::changeResolution(const glm::ivec2& resolutionInput)
    {
        resolution = resolutionInput;
        color      = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT);
        depth      = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32_SFLOAT);
        normal     = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R16G16B16A16_SFLOAT);
        set        = device->createSet(
          {
            { color->asResource(), depth->asResource(), normal->asResource() },
            { sceneDataUbo->asResource() }
        },
          *shader);
        deferred->setInputTextures(getColor(), getDepth(), getNormal());
    }

    void CubeCasterFreecam::prepareRun()
    {
        auto s = cam->getScene();
        sceneDataUbo->uploadToGPU(&s, 1);
        sceneDataUploaded = true;
        deferred->setSceneData(s);
        deferred->setLight(lightData);
    }

    void CubeCasterFreecam::record(Vulcant::VulcantComputeCommand& cmd)
    {
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / 16.0f), (uint32_t)ceil(resolution.y / 16.0f), 1);
        cmd.add(groupCount, *set, *shader);
    }

    Vulcant::VulcantImage& CubeCasterFreecam::getResult()
    {
        return deferred->getTexture();
    }
    Vulcant::VulcantImage& CubeCasterFreecam::getColor()
    {
        return *color;
    }

    Vulcant::VulcantImage& CubeCasterFreecam::getDepth()
    {
        return *depth;
    }

    Vulcant::VulcantImage& CubeCasterFreecam::getNormal()
    {
        return *normal;
    }

    Vulcant::Rendering::Light CubeCasterFreecam::getExampleLight()
    {
        Vulcant::Rendering::Light light = {};

        // Type 1.0 = Omni/Point
        light.type = 1.0f;

        // Position
        light.position[0] = 5.0f;
        light.position[1] = 2.0f;
        light.position[2] = 5.0f;

        // Color and Intensity (Warm Orange)
        light.color[0] = 1.0f;
        light.color[1] = 0.8f;
        light.color[2] = 0.4f;
        light.energy   = 1.5f;

        // Falloff
        light.range       = 20.0f;
        light.attenuation = 1.0f;

        return light;
    }
}