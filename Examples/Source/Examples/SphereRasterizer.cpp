#include "SphereRasterizer.h"

#include "Rendering/DeferredShading.h"
#include "Rendering/Freecam.h"
#include "ShaderLibrary/Example/SphereFrustumCull_comp.h"
#include "ShaderLibrary/Example/SphereAnimation.h"
#include "ShaderLibrary/Example/SphereRasterizer_frag.h"
#include "ShaderLibrary/Example/SphereRasterizer_vert.h"
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
    basePositions.clear();
    velocities.clear();

    spheres.reserve(count);
    sphereInitData.reserve(count);
    basePositions.reserve(count);
    velocities.reserve(count);

    srand(1337);

    for (size_t i = 0; i < count; i++)
    {
        // Generate a 3D spiral / galaxy cluster
        float u     = (float)rand() / (float)RAND_MAX;
        float v     = (float)rand() / (float)RAND_MAX;
        float theta = u * 2.0f * 3.14159265f * 3.0f; // 3 turns
        float dist  = std::pow(v, 0.5f) * 6.0f + 0.2f;

        float x = dist * std::cos(theta) + ((float)rand() / RAND_MAX - 0.5f) * 0.4f;
        float z = dist * std::sin(theta) + ((float)rand() / RAND_MAX - 0.5f) * 0.4f;
        float y = ((float)rand() / RAND_MAX - 0.5f) * (1.2f / (dist * 0.3f + 0.5f));

        glm::vec3 pos(x, y, z);
        basePositions.push_back(pos);

        // Orbit speed inversely proportional to sqrt of distance
        float speed = (0.5f + 0.5f * ((float)rand() / RAND_MAX)) * (1.5f / std::sqrt(dist));
        velocities.push_back(glm::vec3(speed, theta, dist));

        // Color palette based on distance and angle (vibrant nebula gradient)
        float hue = std::fmod(theta * 0.15f + dist * 0.2f, 1.0f);
        float r   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 0.0f / 3.0f));
        float g   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 1.0f / 3.0f));
        float b   = 0.5f + 0.5f * std::cos(6.28318f * (hue + 2.0f / 3.0f));

        uint32_t cr          = (uint32_t)(std::clamp(r, 0.0f, 1.0f) * 255.0f);
        uint32_t cg          = (uint32_t)(std::clamp(g, 0.0f, 1.0f) * 255.0f);
        uint32_t cb          = (uint32_t)(std::clamp(b, 0.0f, 1.0f) * 255.0f);
        uint32_t ca          = 255;
        uint32_t packedColor = cr | (cg << 8) | (cb << 16) | (ca << 24);

        float radiusScale = 0.5f + 0.8f * ((float)rand() / RAND_MAX);
        float sphereRad   = baseRadius * radiusScale;

        SphereData sd;
        sd.center      = pos;
        sd.radius      = sphereRad;
        sd.colorPacked = packedColor;
        spheres.push_back(sd);

        SphereInitData init;
        init.basePosition = pos;
        init.orbitSpeed   = speed;
        init.orbitRadius  = dist;
        init.baseTheta    = theta;
        init.radiusScale  = radiusScale;
        init.packedColor  = packedColor;
        sphereInitData.push_back(init);
    }

    if (sphereInitBuffer)
    {
        sphereInitBuffer->uploadToGPU(sphereInitData.data(), count);
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
    cam = std::make_unique<Vulcant::Rendering::Freecam>(*window, eye, center, up);

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
            frame.animCmd->addBarrier(*frame.spheresBuffer);
            frame.animCmd->endRecord();

            // 1. Frustum Culling Pass
            frame.cullcmd->startRecord();
            frame.cullcmd->add(glm::ivec3((activeSphereCount + 255) / 256, 1, 1), *frame.cullSet, *cullShader);
            frame.cullcmd->addBarrier(*frame.culledSpheresBuffer);
            frame.cullcmd->addBarrier(*frame.indirectDrawBuffer);
            frame.cullcmd->endRecord();

            // 2. Graphic Pass für Frame i
            frame.cmdG->startRecord();
            frame.cmdG->beginRendering(*pipeline);
            frame.cmdG->setViewportAndScissor(glm::uvec2(resolution.x, resolution.y));
            frame.cmdG->drawIndirect(*frame.indirectDrawBuffer, frame.graphicSet.get(), nullptr);
            frame.cmdG->endRendering();
            frame.cmdG->addBarrier(getColor(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
            frame.cmdG->addBarrier(getDepth(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
            frame.cmdG->addBarrier(getNormal(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
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
          ImGui::Text("Rendered Spheres: %d", activeSphereCount);
          ImGui::Separator();

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
          if (cam->keyEvent(key))
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
        frameResources[i].sceneDataUbo  = device->createUniform(1, sizeof(Vulcant::Rendering::SceneData));
        frameResources[i].spheresBuffer = device->createBuffer(MAX_SPHERES, sizeof(SphereData));
        frameResources[i].animUbo       = device->createUniform(1, sizeof(SphereAnimUniforms));
        frameResources[i].sceneDataUbo        = device->createUniform(1, sizeof(Vulcant::Rendering::SceneData));
        frameResources[i].spheresBuffer       = device->createBuffer(MAX_SPHERES, sizeof(SphereData));
        frameResources[i].culledSpheresBuffer = device->createBuffer(MAX_SPHERES, sizeof(SphereData));
        frameResources[i].indirectDrawBuffer = device->createIndirectBuffer(1, sizeof(VkDrawIndirectCommandCPU));
        frameResources[i].cullParamsUbo       = device->createUniform(1, sizeof(CullParams));
    }

    // Shaders
    const auto* cull_slang = SphereFrustumCull_comp_spirv;
    auto        cull_size  = SphereFrustumCull_comp_spirv_sizeInBytes;
    auto        cull_spirv = std::vector<uint32_t>(cull_slang, cull_slang + cull_size / sizeof(uint32_t));

    cullShader = device->createShader(cull_spirv);

    const auto* vert_slang = SphereRasterizer_vert_spirv;
    auto        vert_size  = SphereRasterizer_vert_spirv_sizeInBytes;
    auto        vert_spirv = std::vector<uint32_t>(vert_slang, vert_slang + vert_size / sizeof(uint32_t));

    const auto* frag_slang = SphereRasterizer_frag_spirv;
    auto        frag_size  = SphereRasterizer_frag_spirv_sizeInBytes;
    auto        frag_spirv = std::vector<uint32_t>(frag_slang, frag_slang + frag_size / sizeof(uint32_t));

    const auto* anim_slang = SphereAnimation_spirv;
    auto        anim_size  = SphereAnimation_spirv_sizeInBytes;
    auto        anim_spirv = std::vector<uint32_t>(anim_slang, anim_slang + anim_size / sizeof(uint32_t));

    vertShader = device->createShader(vert_spirv);
    fragShader = device->createShader(frag_spirv);
    animShader = device->createShader(anim_spirv);

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

    // Graphic Pipeline for instanced billboard rasterization
    std::vector<Vulcant::VulcantShader*> shaders          = { vertShader.get(), fragShader.get() };
    std::vector<Vulcant::VulcantImage*>  colorAttachments = { color.get(), normal.get() };

    pipeline = device->createVulcanGraphicPipeline(shaders, colorAttachments, depth.get(), nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        auto& frame = frameResources[i];

        frame.animSet = device->createSet({ { frame.animUbo->asResource(), sphereInitBuffer->asResource(), frame.spheresBuffer->asResource() } }, *animShader);

        // Sets verlinken auf die Frame-spezifischen Buffer
        frame.graphicSet = device->createSet({ { frame.sceneDataUbo->asResource() }, { frame.spheresBuffer->asResource() } }, *vertShader);
        // Graphic set points to sceneDataUbo and culledSpheresBuffer
        frame.graphicSet = device->createSet({ { frame.sceneDataUbo->asResource() }, { frame.culledSpheresBuffer->asResource() } }, *vertShader);

        // Cull set points to sceneDataUbo, spheresBuffer, cullParamsUbo, culledSpheresBuffer, indirectDrawBuffer all in set 0
        frame.cullSet = device->createSet({ { frame.sceneDataUbo->asResource(),
                                              frame.spheresBuffer->asResource(),
                                              frame.cullParamsUbo->asResource(),
                                              frame.culledSpheresBuffer->asResource(),
                                              frame.indirectDrawBuffer->asResource() } },
                                          *cullShader);
    }

    deferred->setInputTextures(getColor(), getDepth(), getNormal());
}

void SphereRasterizer::prepareRun()
{
    auto& frame = frameResources[currentFrame];
    auto  s     = cam->getScene();

    frame.sceneDataUbo->uploadToGPU(&s, 1);

    SphereAnimUniforms animUnif;
    animUnif.time              = elapsedTime;
    animUnif.animSpeed         = animSpeed;
    animUnif.baseRadius        = baseRadius;
    animUnif.activeSphereCount = (uint32_t)activeSphereCount;
    animUnif.pad0 = animUnif.pad1 = animUnif.pad2 = 0;

    frame.animUbo->uploadToGPU(&animUnif, 1);

    CullParams cp{ (uint32_t)activeSphereCount, enableCulling ? 1u : 0u, 0, 0 };
    frame.cullParamsUbo->uploadToGPU(&cp, 1);

    VkDrawIndirectCommandCPU drawCmd{ 6, 0, 0, 0 };
    frame.indirectDrawBuffer->uploadToGPU(&drawCmd, 1);

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
    light.position[1]               = 8.0f;
    light.position[2]               = 0.0f;
    light.color[0]                  = 1.0f;
    light.color[1]                  = 0.95f;
    light.color[2]                  = 0.85f;
    light.energy                    = 2.5f;
    light.range                     = 35.0f;
    light.attenuation               = 1.0f;
    return light;
}
