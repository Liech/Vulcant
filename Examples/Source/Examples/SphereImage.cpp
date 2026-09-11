#include "SphereImage.h"
#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantComputeCommand.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/Wrapper/Window.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "ShaderLibrary/Example/Spherecast.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Vulcant::Examples
{
    void SphereImage::demo()
    {
        auto                              windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();
        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);
        {
            Vulcant::Examples::SphereImage img;
            img.useSlang = true;
            img.createWindow(device, glm::ivec2(800, 600)); // full window render loop
            auto& window = img.getWindow();

            while (!window.isClosed())
            {
                window.tick();
            }
        }
    }

    SphereImage::SphereImage()
    {
        glm::vec3 eye    = glm::vec3(0.0, 0.0, 3.0);
        glm::vec3 center = glm::vec3(0.0, 0.0, 0.0);
        glm::vec3 up     = glm::vec3(0.0, 1.0, 0.0);
        scene.viewMatrix = glm::inverse(glm::lookAt(eye, center, up));
        scene.lightPos   = glm::vec4(5.0, 5.0, 5.0, 1.0f);
    }

    SphereImage::~SphereImage() {}

    Vulcant::VulcantWindow& SphereImage::getWindow()
    {
        return *window;
    }

    void SphereImage::createWindow(Vulcant::VulcantDevice& device, const glm::ivec2& res)
    {
        resolution = res;
        window = device.createWindow(resolution, "Sphere Window");

        prepare(device, res);

        auto rec = [this,&device]()
        {
            cmd = device.createComputeCommand();
            cmd->startRecord();
            cmd->addBarrier(getResult(), Vulcant::VulcantResourceLayout::General);
            record(*cmd);
            cmd->addBarrier(getResult(), Vulcant::VulcantResourceLayout::TransferSrc);
            cmd->endRecord();
        };
        rec();

        window->start([](double) {
            //onLogic
            },
          [this]()
          {//onRender
              prepareRun();
              cmd->runAsync();
              window->blitImage(getResult());
          },
          [this,rec](const glm::ivec2& newResolution)
          {//onResize
              changeResolution(newResolution);
              rec();
          });
    }

    void SphereImage::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolutionInput)
    {
        resolution        = resolutionInput;
        device            = &deviceInput;
        auto shaderSource = getSource();

        const auto* slang      = Spherecast_spirv;
        auto        slang_size = Spherecast_spirv_sizeInBytes;
        auto        spirv      = std::vector<uint32_t>(slang, slang + slang_size / sizeof(uint32_t));

        ubo = device->createBuffer(1, sizeof(PartialSceneData));
        if (useSlang)
            shader = device->createShader(spirv);
        else
            shader = device->createShader(shaderSource);

        changeResolution(resolution);
    }

    void SphereImage::changeResolution(const glm::ivec2& resolutionInput)
    {
        resolution = resolutionInput;
        img        = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT);

        // layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D resultImage;
        // layout(set = 1, binding = 0) buffer SceneBlock{ mat4 viewMatrix; vec4 lightPos; } scene;
        //  call asResource on buffers or images for transfering to gpu in a set
        //{ { Set0Binding0, Set0Binding1,... }, { Set1Binding0, Set1Binding1, ... }, ... }
        set = device->createSet({ { img->asResource() }, { ubo->asResource() } }, *shader);
    }

    void SphereImage::record(Vulcant::VulcantComputeCommand& cmd)
    {
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / 32.0f), (uint32_t)ceil(resolution.y / 32.0f), 1);
        ubo->uploadToGPU(&scene, 1);
        cmd.add(groupCount, *set, *shader);
    }

    float time = 0.0f;
    void  SphereImage::prepareRun()
    {
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / 32.0f), (uint32_t)ceil(resolution.y / 32.0f), 1);

        // upload buffer each frame before invoking shader to display dynamic data
        time += 0.03f;
        glm::vec3 eye    = glm::vec3(0.0, 0.0, 3.0);
        glm::vec3 center = glm::vec3(cos(time) * 0.3f, sin(time) * 0.3f, 0.0); // "screen shake"
        glm::vec3 up     = glm::vec3(0.0, 1.0, 0.0);
        scene.viewMatrix = glm::inverse(glm::lookAt(eye, center, up));

        ubo->uploadToGPU(&scene, 1);
    }

    Vulcant::VulcantImage& SphereImage::getResult()
    {
        return *img;
    }

    std::string SphereImage::getSource()
    {
        return R"(
#version 450

#define WORKGROUP_SIZE 32
layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1 ) in;

layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D resultImage;

// Uniform block for dynamic input
layout(set = 1, binding = 0) buffer SceneBlock {
    mat4 viewMatrix;
    vec4 lightPos;
} scene;

struct Ray {
    vec3 origin;
    vec3 direction;
};

float hit_sphere(vec3 center, float radius, Ray r) {
    vec3 oc = r.origin - center;
    float b = dot(oc, r.direction);
    float c = dot(oc, oc) - radius * radius;
    float h = b*b - c;
    if (h < 0.0) return -1.0;
    return -b - sqrt(h);
}

void main() {
    ivec2 dims = imageSize(resultImage);
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    if(pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) return;

    vec2 uv = (vec2(pixel_coords) / vec2(dims)) * 2.0 - 1.0;
    uv.x *= float(dims.x) / float(dims.y);

    // 1. Generate local ray
    vec3 rayOrigin = vec3(0.0, 0.0, 0.0);
    vec3 rayDir = normalize(vec3(uv, -1.5)); // -1.5 is the focal length

    // 2. Transform ray by Camera Matrix
    // Origin is translated, Direction is only rotated (w=0)
    vec3 worldOrigin = (scene.viewMatrix * vec4(rayOrigin, 1.0)).xyz;
    vec3 worldDir = normalize((scene.viewMatrix * vec4(rayDir, 0.0)).xyz);

    Ray ray = Ray(worldOrigin, worldDir);

    // Scene properties
    vec3 sphereCenter = vec3(0.0, 0.0, 0.0);
    float radius = 0.5;
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 0.0); // Transparent background

    float t = hit_sphere(sphereCenter, radius, ray);
    if (t > 0.0) {
        vec3 hitPoint = ray.origin + t * ray.direction;
        vec3 normal = normalize(hitPoint - sphereCenter);
        
        // Lighting logic using uniform light position
        vec3 lightDir = normalize(scene.lightPos.xyz - hitPoint);
        float diff = max(dot(normal, lightDir), 0.0);
        
        vec3 sphereColor = vec3(0.8, 0.2, 0.2);
        vec3 ambient = sphereColor * 0.1;
        finalColor = vec4(ambient + (sphereColor * diff), 1.0); // Opaque sphere
    }

    imageStore(resultImage, pixel_coords, finalColor);
}
)";
    }
}