#include "SphereRasterizer.h"

#include "Examples/SphereRasterizerScenes/ConcentricRingsScene.h"
#include "Examples/SphereRasterizerScenes/CubeGridScene.h"
#include "Examples/SphereRasterizerScenes/FloatingSpheresScene.h"
#include "Examples/SphereRasterizerScenes/HillLandscapeScene.h"
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

static const std::string cullClearGlsl = R"(
#version 450
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

struct DrawIndirectCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

layout(std430, binding = 4) buffer IndirectCommand {
    DrawIndirectCommand gIndirectCommand[];
};

layout(std430, binding = 5) buffer SortBuffer {
    uint binCounts[256];
    uint binOffsets[256];
};

void main() {
    uint tid = gl_GlobalInvocationID.x;
    if (tid < 256) {
        binCounts[tid] = 0;
        binOffsets[tid] = 0;
    }
    if (tid == 0) {
        gIndirectCommand[0].vertexCount = 6;
        gIndirectCommand[0].instanceCount = 0;
        gIndirectCommand[0].firstVertex = 0;
        gIndirectCommand[0].firstInstance = 0;
    }
}
)";

static const std::string cullCountGlsl = R"(
#version 450
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

struct SphereData {
    vec3 center;
    float radius;
    uint colorPacked;
    uint pad0;
};

struct DrawIndirectCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct CullParams {
    uint activeSphereCount;
    uint enableCulling;
    uint enableZSorting;
    uint pad0;
};

layout(std140, binding = 0) uniform SceneDataUbo {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invProjectionMatrix;
    mat4 invViewMatrix;
    vec4 cameraPos;
    vec2 resolution;
    float nearPlane;
    float farPlane;
} scene;

layout(std430, binding = 1) readonly buffer Spheres {
    SphereData gSpheres[];
};

layout(std140, binding = 2) uniform CullParamsUbo {
    CullParams cullParams;
};

layout(std430, binding = 3) writeonly buffer CulledSpheres {
    SphereData gCulledSpheres[];
};

layout(std430, binding = 4) buffer IndirectCommand {
    DrawIndirectCommand gIndirectCommand[];
};

layout(std430, binding = 5) buffer SortBuffer {
    uint binCounts[256];
    uint binOffsets[256];
};

bool isSphereVisible(SphereData sphere) {
    if (cullParams.enableCulling == 0) return true;
    float r = abs(sphere.radius);
    mat4 vp = scene.projectionMatrix * scene.viewMatrix;
    vec4 planes[6];
    planes[0] = vp[3] + vp[0];
    planes[1] = vp[3] - vp[0];
    planes[2] = vp[3] + vp[1];
    planes[3] = vp[3] - vp[1];
    planes[4] = vp[2];
    planes[5] = vp[3] - vp[2];

    for (int i = 0; i < 6; ++i) {
        vec4 p = planes[i];
        float len = length(p.xyz);
        p /= len;
        float dist = dot(p.xyz, sphere.center) + p.w;
        if (dist < -r) return false;
    }
    return true;
}

void main() {
    uint sphereID = gl_GlobalInvocationID.x;
    if (sphereID >= cullParams.activeSphereCount) return;

    SphereData sphere = gSpheres[sphereID];
    if (!isSphereVisible(sphere)) return;

    if (cullParams.enableZSorting == 0) {
        uint appendIndex = atomicAdd(gIndirectCommand[0].instanceCount, 1);
        gCulledSpheres[appendIndex] = sphere;
    } else {
        atomicAdd(gIndirectCommand[0].instanceCount, 1);
        vec3 centerView = (scene.viewMatrix * vec4(sphere.center, 1.0)).xyz;
        float dist = -centerView.z;
        float minDepth = scene.nearPlane > 0.0 ? scene.nearPlane : 0.1;
        float maxDepth = scene.farPlane > minDepth ? scene.farPlane : 1000.0;
        float normDepth = clamp((dist - minDepth) / (maxDepth - minDepth), 0.0, 0.9999);
        uint binIndex = uint(normDepth * 256.0);
        atomicAdd(binCounts[binIndex], 1);
    }
}
)";

static const std::string cullPrefixGlsl = R"(
#version 450
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 5) buffer SortBuffer {
    uint binCounts[256];
    uint binOffsets[256];
};

shared uint sharedCounts[256];

void main() {
    uint tid = gl_LocalInvocationID.x;
    sharedCounts[tid] = binCounts[tid];
    barrier();

    uint sum = 0;
    for (uint i = 0; i < tid; ++i) {
        sum += sharedCounts[i];
    }
    binOffsets[tid] = sum;
}
)";

static const std::string cullScatterGlsl = R"(
#version 450
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

struct SphereData {
    vec3 center;
    float radius;
    uint colorPacked;
    uint pad0;
};

struct CullParams {
    uint activeSphereCount;
    uint enableCulling;
    uint enableZSorting;
    uint pad0;
};

layout(std140, binding = 0) uniform SceneDataUbo {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invProjectionMatrix;
    mat4 invViewMatrix;
    vec4 cameraPos;
    vec2 resolution;
    float nearPlane;
    float farPlane;
} scene;

layout(std430, binding = 1) readonly buffer Spheres {
    SphereData gSpheres[];
};

layout(std140, binding = 2) uniform CullParamsUbo {
    CullParams cullParams;
};

layout(std430, binding = 3) writeonly buffer CulledSpheres {
    SphereData gCulledSpheres[];
};

layout(std430, binding = 5) buffer SortBuffer {
    uint binCounts[256];
    uint binOffsets[256];
};

bool isSphereVisible(SphereData sphere) {
    if (cullParams.enableCulling == 0) return true;
    float r = abs(sphere.radius);
    mat4 vp = scene.projectionMatrix * scene.viewMatrix;
    vec4 planes[6];
    planes[0] = vp[3] + vp[0];
    planes[1] = vp[3] - vp[0];
    planes[2] = vp[3] + vp[1];
    planes[3] = vp[3] - vp[1];
    planes[4] = vp[2];
    planes[5] = vp[3] - vp[2];

    for (int i = 0; i < 6; ++i) {
        vec4 p = planes[i];
        float len = length(p.xyz);
        p /= len;
        float dist = dot(p.xyz, sphere.center) + p.w;
        if (dist < -r) return false;
    }
    return true;
}

void main() {
    uint sphereID = gl_GlobalInvocationID.x;
    if (sphereID >= cullParams.activeSphereCount) return;

    SphereData sphere = gSpheres[sphereID];
    if (!isSphereVisible(sphere)) return;

    vec3 centerView = (scene.viewMatrix * vec4(sphere.center, 1.0)).xyz;
    float dist = -centerView.z;
    float minDepth = scene.nearPlane > 0.0 ? scene.nearPlane : 0.1;
    float maxDepth = scene.farPlane > minDepth ? scene.farPlane : 1000.0;
    float normDepth = clamp((dist - minDepth) / (maxDepth - minDepth), 0.0, 0.9999);
    uint binIndex = uint(normDepth * 256.0);
    uint destIndex = atomicAdd(binOffsets[binIndex], 1);
    gCulledSpheres[destIndex] = sphere;
}
)";

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

            // 1. Frustum Culling & Depth Bin Sorting Pass
            frame.cullcmd->startRecord();
            if (enableZSorting)
            {
                frame.cullcmd->add(glm::ivec3(1, 1, 1), *frame.cullSet, *cullClearShader);
                frame.cullcmd->addBarrier(*frame.sortBuffer);
                frame.cullcmd->addBarrier(*frame.indirectDrawBuffer);

                frame.cullcmd->add(glm::ivec3((activeSphereCount + 255) / 256, 1, 1), *frame.cullSet, *cullCountShader);
                frame.cullcmd->addBarrier(*frame.sortBuffer);
                frame.cullcmd->addBarrier(*frame.indirectDrawBuffer);

                frame.cullcmd->add(glm::ivec3(1, 1, 1), *frame.cullSet, *cullPrefixShader);
                frame.cullcmd->addBarrier(*frame.sortBuffer);

                frame.cullcmd->add(glm::ivec3((activeSphereCount + 255) / 256, 1, 1), *frame.cullSet, *cullScatterShader);
                frame.cullcmd->addBarrier(*frame.culledSpheresBuffer);
            }
            else
            {
                frame.cullcmd->add(glm::ivec3((activeSphereCount + 255) / 256, 1, 1), *frame.cullSet, *cullShader);
                frame.cullcmd->addBarrier(*frame.culledSpheresBuffer);
                frame.cullcmd->addBarrier(*frame.indirectDrawBuffer);
            }
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

          if (ImGui::Checkbox("Enable Front-to-Back Z-Sorting", &enableZSorting))
          {
              rec();
          }

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
        frameResources[i].sceneDataUbo        = device->createUniform(1, sizeof(Vulcant::Rendering::SceneData));
        frameResources[i].spheresBuffer       = device->createBuffer(MAX_SPHERES, sizeof(SphereData));
        frameResources[i].animUbo             = device->createUniform(1, sizeof(SphereAnimUniforms));
        frameResources[i].culledSpheresBuffer = device->createBuffer(MAX_SPHERES, sizeof(SphereData));
        frameResources[i].indirectDrawBuffer  = device->createIndirectBuffer(1, sizeof(VkDrawIndirectCommandCPU));
        frameResources[i].cullParamsUbo       = device->createUniform(1, sizeof(CullParams));
        frameResources[i].sortBuffer          = device->createBuffer(512, sizeof(uint32_t));
    }

    // Shaders
    const auto* cull_slang = SphereFrustumCull_comp_spirv;
    auto        cull_size  = SphereFrustumCull_comp_spirv_sizeInBytes;
    auto        cull_spirv = std::vector<uint32_t>(cull_slang, cull_slang + cull_size / sizeof(uint32_t));

    cullShader        = device->createShader(cull_spirv);
    cullClearShader   = device->createShader(cullClearGlsl);
    cullCountShader   = device->createShader(cullCountGlsl);
    cullPrefixShader  = device->createShader(cullPrefixGlsl);
    cullScatterShader = device->createShader(cullScatterGlsl);

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

        // Cull set points to sceneDataUbo, spheresBuffer, cullParamsUbo, culledSpheresBuffer, indirectDrawBuffer, sortBuffer all in set 0
        frame.cullSet = device->createSet({ { frame.sceneDataUbo->asResource(),
                                              frame.spheresBuffer->asResource(),
                                              frame.cullParamsUbo->asResource(),
                                              frame.culledSpheresBuffer->asResource(),
                                              frame.indirectDrawBuffer->asResource(),
                                              frame.sortBuffer->asResource() } },
                                          *cullCountShader);
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

    CullParams cp{ (uint32_t)activeSphereCount, enableCulling ? 1u : 0u, enableZSorting ? 1u : 0u, 0 };
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

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>

TEST_CASE("SphereRasterizer CullParams and Depth Bin Sorting Setup", "[SphereRasterizer]")
{
    REQUIRE(sizeof(CullParams) == 16);

    CullParams cp{};
    cp.activeSphereCount = 10000;
    cp.enableCulling     = 1;
    cp.enableZSorting   = 1;
    cp.pad0             = 0;

    REQUIRE(cp.activeSphereCount == 10000);
    REQUIRE(cp.enableCulling == 1);
    REQUIRE(cp.enableZSorting == 1);

    SphereRasterizer app;
    REQUIRE(app.getName() == "SphereRasterizer");
    REQUIRE(!app.getDescription().empty());
}
#endif
