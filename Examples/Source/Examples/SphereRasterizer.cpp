#include "SphereRasterizer.h"

#include "Examples/SphereRasterizerScenes/ConcentricRingsScene.h"
#include "Examples/SphereRasterizerScenes/CubeGridScene.h"
#include "Examples/SphereRasterizerScenes/FloatingSpheresScene.h"
#include "Examples/SphereRasterizerScenes/HillLandscapeScene.h"
#include "Rendering/DeferredShading.h"
#include "Rendering/Camera.h"
#include "Rendering/Freecam.h"
#include "Rendering/ArcCam.h"
#include "ShaderLibrary/Example/SphereAnimation.h"
#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantComputeCommand.h"
#include "Vulcant/Interface/VulcantGraphicCommand.h"
#include "Vulcant/Interface/VulcantGraphicPipeline.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantInput.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "Vulcant/Interface/VulcantUi.h"
#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "Vulcant/Wrapper/Window.h"
#include <algorithm>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>

void SphereRasterizer::demo()
{
    auto                              windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();
    Vulcant::VulcantV::VulcantVDevice device(windowExtensions);
    {
        SphereRasterizer app;
        app.createWindow(device, glm::ivec2(3840, 2130));
        auto& window = app.getWindow();

        while (!window.isClosed())
        {
            window.tick();
        }
    }
}

SphereRasterizer::SphereRasterizer()
{
    scenes.push_back(std::make_unique<Vulcant::Examples::HillLandscapeScene>());
    scenes.push_back(std::make_unique<Vulcant::Examples::FloatingSpheresScene>());
    scenes.push_back(std::make_unique<Vulcant::Examples::CubeGridScene>());
    scenes.push_back(std::make_unique<Vulcant::Examples::ConcentricRingsScene>());

    lightData = { getExampleLight() };
    regenerateSpheres(activeSphereCount);
}

SphereRasterizer::~SphereRasterizer() {}

Vulcant::VulcantWindow& SphereRasterizer::getWindow()
{
    return *window;
}

void SphereRasterizer::regenerateSpheres(size_t count)
{
    spheres.clear();
    sphereInitData.clear();

    if (!scenes.empty() && currentSceneIndex >= 0 && currentSceneIndex < (int)scenes.size())
    {
        scenes[currentSceneIndex]->generate(count, baseRadius, spheres, sphereInitData);
    }

    if (sphereInitBuffer)
    {
        sphereInitBuffer->uploadToGPU(sphereInitData.data(), count);
    }

    for (size_t i = 0; i < frameResources.size(); i++)
    {
        if (frameResources[i].sphereRenderer)
        {
            frameResources[i].sphereRenderer->setActiveSphereCount(count);
        }
        if (frameResources[i].cubeRenderer)
        {
            frameResources[i].cubeRenderer->setActiveCubeCount(count);
        }
    }
}

void SphereRasterizer::updateSpheres(float time, float dt)
{
    // Math loop offloaded to GPU compute shader (SphereAnimation.exe.slang)
}

void SphereRasterizer::syncAnimationBuffers()
{
    // Wait for all frame commands to complete
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& frame = frameResources[i];
        if (frame.animCmd) frame.animCmd->wait();
        if (frame.cullcmd) frame.cullcmd->wait();
        if (frame.cmdG) frame.cmdG->wait();
        if (frame.defcmd) frame.defcmd->wait();
        if (frame.uiCmdG) frame.uiCmdG->wait();
    }

    // Run animCmd for all frames using the exact same static elapsedTime
    SphereAnimUniforms animUnif;
    animUnif.time              = elapsedTime;
    animUnif.animSpeed         = animSpeed;
    animUnif.baseRadius        = baseRadius;
    animUnif.activeSphereCount = (uint32_t)activeSphereCount;
    animUnif.pad0 = animUnif.pad1 = animUnif.pad2 = 0;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& frame = frameResources[i];
        frame.animUbo->uploadToGPU(&animUnif, 1);
        frame.animCmd->runAsync();
        frame.animCmd->wait();
    }
}

void SphereRasterizer::createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res)
{
    resolution = res;
    window     = deviceInput.createWindow(resolution, "High-Performance Dynamic Sphere Rasterizer");
    glm::vec3 eye(0.0f, 4.0f, 8.0f);
    glm::vec3 center(0.0f, 0.0f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    freecam = std::make_unique<Vulcant::Rendering::Freecam>(*window, eye, center, up);
    arccam  = std::make_unique<Vulcant::Rendering::ArcCam>(*window, eye, center, up);
    cam     = freecam.get();

    prepare(deviceInput, resolution);


    auto rec = [this]()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto& frame = frameResources[i];
            if (frame.animCmd)
                frame.animCmd->wait();
            if (frame.cullcmd)
                frame.cullcmd->wait();
            if (frame.cmdG)
                frame.cmdG->wait();
            if (frame.defcmd)
                frame.defcmd->wait();
            if (frame.uiCmdG)
                frame.uiCmdG->wait();
        }


        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto& frame = frameResources[i];

            // 2. Graphic Pass für Frame i
            frame.animCmd = device->createComputeCommand();
            frame.cmdG    = device->createGraphicCommand();
            frame.uiCmdG  = device->createGraphicCommand();
            frame.defcmd  = device->createComputeCommand();
            frame.cullcmd = device->createComputeCommand();

            // 1. Sphere Animation Compute Pass
            frame.animCmd->startRecord();
            uint32_t groupCountX = (uint32_t)std::ceil((float)activeSphereCount / 256.0f);
            frame.animCmd->add(glm::ivec3(groupCountX, 1, 1), *frame.animSet, *animShader);
            frame.animCmd->addBarrier(frame.sphereRenderer->getSpheresBuffer());
            frame.animCmd->endRecord();

            // 1. Frustum Culling Pass
            frame.cullcmd->startRecord();
            if (renderMode == RenderMode::Spheres)
            {
                frame.sphereRenderer->recordCull(*frame.cullcmd);
            }
            else
            {
                frame.cubeRenderer->recordCull(*frame.cullcmd);
            }
            frame.cullcmd->endRecord();

            // 2. Graphic Pass für Frame i
            frame.cmdG->startRecord();
            if (renderMode == RenderMode::Spheres)
            {
                frame.sphereRenderer->record(*frame.cmdG);
            }
            else
            {
                frame.cubeRenderer->record(*frame.cmdG);
            }
            frame.cmdG->endRecord();

            // 3. Deferred Lighting Pass
            // 3. Deferred Shading Pass für Frame i
            frame.defcmd->startRecord();
            frame.defcmd->addBarrier(deferred->getTexture(), Vulcant::VulcantResourceLayout::General);
            deferred->record(*frame.defcmd);
            frame.defcmd->addBarrier(deferred->getTexture(), Vulcant::VulcantResourceLayout::TransferSrc);
            frame.defcmd->endRecord();
        }
    };

    rec();

    window->start(
      [this, rec](double delta) // onLogic
      {
          float dt = (float)delta;
          if (animate)
          {
              elapsedTime += dt;
          }

          cam->tick(delta);
          updateSpheres(elapsedTime, dt);

          ui->newFrame();

          ImGui::Begin("Sphere Rasterizer Controls");
          float frameTimeMs = ImGui::GetIO().Framerate > 0.0f ? (1000.0f / ImGui::GetIO().Framerate) : 0.0f;
          ImGui::Text("Frametime: %.2f ms", frameTimeMs);
          ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
          ImGui::Text("Rendered Instances: %d", activeSphereCount);
          ImGui::Separator();

          ImGui::Text("Render Mode:");
          int modeInt = (renderMode == RenderMode::Spheres) ? 0 : 1;
          if (ImGui::RadioButton("Spheres", &modeInt, 0))
          {
              if (renderMode != RenderMode::Spheres)
              {
                  renderMode = RenderMode::Spheres;
                  rec();
              }
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("Cubes", &modeInt, 1))
          {
              if (renderMode != RenderMode::Cubes)
              {
                  renderMode = RenderMode::Cubes;
                  rec();
              }
          }
          ImGui::Separator();

          ImGui::Text("Camera Mode:");
          int camInt = (cameraType == CameraType::Freecam) ? 0 : 1;
          if (ImGui::RadioButton("Freecam (WASD + Mouse)", &camInt, 0))
          {
              if (cameraType != CameraType::Freecam)
              {
                  cameraType = CameraType::Freecam;
                  freecam->setEye(arccam->getEye());
                  freecam->setTarget(arccam->getTarget());
                  freecam->setUp(arccam->getUp());
                  freecam->setActive(arccam->isActive());
                  cam = freecam.get();
              }
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("ArcCam (Orbit / Pan)", &camInt, 1))
          {
              if (cameraType != CameraType::ArcCam)
              {
                  cameraType = CameraType::ArcCam;
                  arccam->setEye(freecam->getEye());
                  arccam->setTarget(freecam->getTarget());
                  arccam->setUp(freecam->getUp());
                  arccam->setActive(freecam->isActive());
                  cam = arccam.get();
              }
          }
          ImGui::Separator();

          if (!scenes.empty())
          {
              if (ImGui::BeginCombo("Scene", scenes[currentSceneIndex]->getName().c_str()))
              {
                  for (int i = 0; i < (int)scenes.size(); i++)
                  {
                      bool isSelected = (currentSceneIndex == i);
                      if (ImGui::Selectable(scenes[i]->getName().c_str(), isSelected))
                      {
                          if (currentSceneIndex != i)
                          {
                              currentSceneIndex = i;
                              regenerateSpheres(activeSphereCount);
                              rec();
                              if (!animate)
                              {
                                  syncAnimationBuffers();
                              }
                          }
                      }
                      if (isSelected)
                      {
                          ImGui::SetItemDefaultFocus();
                      }
                  }
                  ImGui::EndCombo();
              }
              ImGui::Separator();
          }

          static const int countPresets[] = { 10000, 50000, 100000, 250000, 500000, 1000000, 3000000, 4000000, 10000000, MAX_SPHERES };
          ImGui::Text("Sphere Count Presets:");
          for (int preset : countPresets)
          {
              char label[32];
              snprintf(label, sizeof(label), "%dk", preset / 1000);
              if (ImGui::Button(label))
              {
                  activeSphereCount = preset;
                  regenerateSpheres(activeSphereCount);
                  rec();
              }
              ImGui::SameLine();
          }
          ImGui::NewLine();

          if (ImGui::Checkbox("Animate Dynamic Spheres", &animate))
          {
              if (!animate)
              {
                  syncAnimationBuffers();
              }
          }
          if (animate)
          {
              ImGui::SliderFloat("Animation Speed", &animSpeed, 0.0f, 5.0f);
          }

          ImGui::Checkbox("Enable Frustum Culling", &enableCulling);

          ImGui::SliderFloat("Base Radius", &baseRadius, 0.01f, 0.25f);

          ImGui::Separator();
          // ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);
          ImGui::End();
      },
      [this]() // onRender
      {
          auto& frame = frameResources[currentFrame];

          // 1. Warten, bis genau DIESER Frame-Slot auf der GPU fertig abgearbeitet wurde
          frame.animCmd->wait();
          frame.cullcmd->wait();
          frame.cmdG->wait();
          frame.defcmd->wait();
          frame.uiCmdG->wait();

          // 2. Jetzt gefahrlos Uniform-Daten für diesen Frame hochladen
          prepareRun();

          // 3. Render-Commands starten
          if (animate)
          {
              frame.animCmd->runAsync();
          }
          frame.cullcmd->runAsync();
          frame.cmdG->runAsync();
          frame.defcmd->runAsync();

          frame.uiCmdG->startRecord();
          ui->record(*frame.uiCmdG, getResult());
          frame.uiCmdG->endRecord();
          frame.uiCmdG->runAsync();

          window->blitImage(getResult());

          // 4. Weiterblättern zum nächsten Frame-Index
          currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
      },
      [this, rec](const glm::ivec2& newResolution) // onResize
      {
          changeResolution(newResolution);
          rec();
      });

    window->getInput().setCallback(
      [this](const Vulcant::VulcantInputValue& key)
      {
          if (key == Vulcant::VulcantInputValue::C)
          {
              if (cameraType == CameraType::Freecam)
              {
                  cameraType = CameraType::ArcCam;
                  arccam->setEye(freecam->getEye());
                  arccam->setTarget(freecam->getTarget());
                  arccam->setUp(freecam->getUp());
                  arccam->setActive(freecam->isActive());
                  cam = arccam.get();
              }
              else
              {
                  cameraType = CameraType::Freecam;
                  freecam->setEye(arccam->getEye());
                  freecam->setTarget(arccam->getTarget());
                  freecam->setUp(arccam->getUp());
                  freecam->setActive(arccam->isActive());
                  cam = freecam.get();
              }
              return;
          }

          if (cam && cam->keyEvent(key))
              return;
      });
}

void SphereRasterizer::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res)
{
    device     = &deviceInput;
    resolution = res;

    sphereInitBuffer = device->createBuffer(MAX_SPHERES, sizeof(SphereInitData));
    sphereInitBuffer->uploadToGPU(sphereInitData.data(), activeSphereCount);

    frameResources.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        frameResources[i].animUbo        = device->createUniform(1, sizeof(SphereAnimUniforms));
        frameResources[i].sphereRenderer = std::make_unique<Vulcant::Rendering::SphereRenderer>(*device, glm::ivec2(0, 0), MAX_SPHERES);
        frameResources[i].sphereRenderer->setActiveSphereCount(activeSphereCount);

        frameResources[i].cubeRenderer   = std::make_unique<Vulcant::Rendering::CubeRenderer>(*device, glm::ivec2(0, 0), MAX_SPHERES);
        frameResources[i].cubeRenderer->setCubesBuffer(frameResources[i].sphereRenderer->getSpheresBuffer(), activeSphereCount);
    }

    const auto* anim_slang = SphereAnimation_spirv;
    auto        anim_size  = SphereAnimation_spirv_sizeInBytes;
    auto        anim_spirv = std::vector<uint32_t>(anim_slang, anim_slang + anim_size / sizeof(uint32_t));
    animShader             = device->createShader(anim_spirv);

    deferred = std::make_unique<Vulcant::Rendering::DeferredShading>();
    deferred->prepare(*device);

    ui = device->createUi(*window, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT, false);

    changeResolution(resolution);
}

void SphereRasterizer::changeResolution(const glm::ivec2& newResolution)
{
    resolution = newResolution;

    // G-Buffer attachments
    color  = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT);
    normal = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R16G16B16A16_SFLOAT);
    depth  = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& frame = frameResources[i];
        frame.sphereRenderer->setOutputTextures(*color, *depth, *normal);
        frame.sphereRenderer->setResolution(resolution);

        frame.cubeRenderer->setOutputTextures(*color, *depth, *normal);
        frame.cubeRenderer->setResolution(resolution);

        frame.animSet = device->createSet(
            { { frame.animUbo->asResource(), sphereInitBuffer->asResource(), frame.sphereRenderer->getSpheresBuffer().asResource() } },
            *animShader);
    }

    deferred->setInputTextures(getColor(), getDepth(), getNormal());
}

void SphereRasterizer::prepareRun()
{
    auto& frame = frameResources[currentFrame];
    auto  s     = cam->getScene();

    if (renderMode == RenderMode::Spheres)
    {
        frame.sphereRenderer->setSceneData(s);
        frame.sphereRenderer->setCullingEnabled(enableCulling);
    }
    else
    {
        frame.cubeRenderer->setSceneData(s);
        frame.cubeRenderer->setCullingEnabled(enableCulling);
    }

    SphereAnimUniforms animUnif;
    animUnif.time              = elapsedTime;
    animUnif.animSpeed         = animSpeed;
    animUnif.baseRadius        = baseRadius;
    animUnif.activeSphereCount = (uint32_t)activeSphereCount;
    animUnif.pad0 = animUnif.pad1 = animUnif.pad2 = 0;

    frame.animUbo->uploadToGPU(&animUnif, 1);

    deferred->setSceneData(s);
    deferred->setLight(lightData);
}

void SphereRasterizer::record(Vulcant::VulcantComputeCommand& cmd) {}

Vulcant::VulcantImage& SphereRasterizer::getResult()
{
    return deferred->getTexture();
}

Vulcant::VulcantImage& SphereRasterizer::getColor()
{
    return *color;
}

Vulcant::VulcantImage& SphereRasterizer::getDepth()
{
    return *depth;
}

Vulcant::VulcantImage& SphereRasterizer::getNormal()
{
    return *normal;
}

Vulcant::Rendering::Light SphereRasterizer::getExampleLight()
{
    Vulcant::Rendering::Light light = {};
    light.type                      = 1.0f; // Point light
    light.position[0]               = 0.0f;
    light.position[1]               = 80.0f;
    light.position[2]               = 0.0f;
    light.color[0]                  = 1.0f;
    light.color[1]                  = 0.95f;
    light.color[2]                  = 0.85f;
    light.energy                    = 2.5f;
    light.range                     = 2500.0f;
    light.attenuation               = 1.0f;
    return light;
}
