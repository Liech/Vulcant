#include "MandelbrotImage.h"

#include "Vulcant/Interface/VulcantComputeCommand.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/Wrapper/Window.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "ShaderLibrary/Example/Mandelbrot.h"
#include <stdexcept>

namespace Vulcant::Examples
{
    void MandelbrotImage::demo()
    {
        auto                               windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions(); // GLWF is optional
        Vulcant::VulcantV::VulcantVDevice  device(windowExtensions);                                           // Create specific device implementation (see also VulcantGDevice)
        {
            Vulcant::Examples::MandelbrotImage img;
            img.useSlang = true;                            // slang is optional, glsl implementation below
            img.createWindow(device, glm::ivec2(800, 600)); // full window render loop
            auto& window = img.getWindow();

            while (!window.isClosed())
            {
                window.tick();
            }
        }
    }

    MandelbrotImage::MandelbrotImage() {}
    MandelbrotImage::~MandelbrotImage() {}

    Vulcant::VulcantWindow& MandelbrotImage::getWindow()
    {
        return *window;
    }

    void MandelbrotImage::createWindow(Vulcant::VulcantDevice& device, const glm::ivec2& inputResolution)
    {
        resolution = inputResolution;
        // optional window display. see VulcantImage::saveRenderedImage or VulcantBuffer::downloadFromGPU for "headless" results
        window = device.createWindow(resolution, "Mandelbrot Window");

        prepare(device, inputResolution); // compile shader, prepare set (set is a parameter for shaders)

        auto rec = [this, &device]()
        {
            // define the GPU queue
            cmd = device.createComputeCommand();
            cmd->startRecord();
            cmd->addBarrier(getResult(), Vulcant::VulcantResourceLayout::General);     // Images need to be set for a usage. In this case writing
            record(*cmd);                                                              // add pairs of shaders and sets to the gpu queue
            cmd->addBarrier(getResult(), Vulcant::VulcantResourceLayout::TransferSrc); // Images need to be set for a usage. In this case copy/paste
            cmd->endRecord();
        };
        rec();

        window->start(
          [](double) {
            // onLogic
            },
          [this]()
          {//onRender
              prepareRun();    // usually upload data to buffer. In this example: nothing
              cmd->runAsync(); // execute command (images are guarded by semaphores respected in other runAsyncs and window::blitImage. so cmd->runAsync(); cmd2->runAsync() when the same image is used
                               // is safe)
              window->blitImage(getResult()); // copy/paste result image onto window
          },
          [this,rec](const glm::ivec2& newResolution)
          {//onResize
              // on resolution change only shaders stay valid:
              changeResolution(newResolution);
              rec();
          });
    }

    void MandelbrotImage::prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& inputResolution)
    {
        device                   = &deviceInput;
        resolution               = inputResolution;
        auto        shaderSource = Vulcant::Examples::MandelbrotImage::getSource(); // glsl
        const auto* slang        = Mandelbrot_spirv;
        auto        slang_size   = Mandelbrot_spirv_sizeInBytes;
        auto        spirv        = std::vector<uint32_t>(slang, slang + slang_size / sizeof(uint32_t)); // compiled shader code

        if (useSlang)
            shader = device->createShader(spirv);
        else
            shader = device->createShader(shaderSource);
        changeResolution(resolution);
    }

    void MandelbrotImage::changeResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;
        // create 2d image
        img = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT);

        // transfers data to defined layout
        // layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D resultImage;
        //{ { Set0Binding0, Set0Binding1,... }, { Set1Binding0, Set1Binding1, ... }, ... }
        set = device->createSet({ { img->asResource() } }, *shader);
    }

    void MandelbrotImage::record(Vulcant::VulcantComputeCommand& cmd)
    {
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / float(WORKGROUP_SIZE)), (uint32_t)ceil(resolution.y / float(WORKGROUP_SIZE)), 1); // amount of threads
        cmd.add(groupCount, *set, *shader);                                                                                                                // adds to gpu queue
    }

    void MandelbrotImage::prepareRun() {}

    Vulcant::VulcantImage& MandelbrotImage::getResult()
    {
        return *img;
    }

    // glsl shader coded
    // slang shaeder code is in ShaderLibrary project included with #include "ShaderLibrary/Example/Mandelbrot.h", which is compiled by a cmake postprocessing of shaderlibrary
    std::string MandelbrotImage::getSource()
    {
        return R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

#define WORKGROUP_SIZE 32
layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1 ) in;

// NEU: Statt des Buffers nutzen wir ein image2D. 
// rgba32f passt zu deinem vec4-Output.
layout(set = 0, binding = 0, rgba32f) uniform writeonly image2D resultImage;

void main() {
  // Die Dimensionen holen wir uns jetzt direkt vom Image statt via #define
  ivec2 dims = imageSize(resultImage);
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

  // Bounds-Check
  if(pixel_coords.x >= dims.x || pixel_coords.y >= dims.y)
    return;

  float x = float(pixel_coords.x) / float(dims.x);
  float y = float(pixel_coords.y) / float(dims.y);

  vec2 uv = vec2(x,y);
  float n = 0.0;
  vec2 c = vec2(-.445, 0.0) + (uv - 0.5)*(2.0 + 1.7*0.2);
  vec2 z = vec2(0.0);
  const int M = 128;
  for (int i = 0; i < M; i++)
  {
    z = vec2(z.x*z.x - z.y*z.y, 2.*z.x*z.y) + c;
    if (dot(z, z) > 2.0) break;
    n++;
  }
          
  float t = float(n) / float(M);
  vec3 d = vec3(0.3, 0.3 ,0.5);
  vec3 e = vec3(-0.2, -0.3 ,-0.5);
  vec3 f = vec3(2.1, 2.0, 3.0);
  vec3 g = vec3(0.0, 0.1, 0.0);
  vec4 color = vec4( d + e*cos( 6.28318*(f*t+g) ) ,1.0);
          
  // NEU: Speichern mittels imageStore statt Array-Index
  imageStore(resultImage, pixel_coords, color);
}
)";
    }
}