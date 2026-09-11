#include "DeferredShading.h"
#include <stdexcept>
#include <string_view>

#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantComputeCommand.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "ShaderLibrary/DeferredShader.h"

namespace Vulcant::Rendering
{
    DeferredShading::DeferredShading() {}

    DeferredShading::~DeferredShading() {}

    void DeferredShading::prepare(Vulcant::VulcantDevice& deviceInput)
    {
        device                 = &deviceInput;
        const auto* slang      = DeferredShader_spirv;
        auto        slang_size = DeferredShader_spirv_sizeInBytes;
        auto        spirv      = std::vector<uint32_t>(slang, slang + slang_size / sizeof(uint32_t));

        shader       = device->createShader(spirv);
        sceneDataUbo = device->createUniform(1, sizeof(SceneData));
        LightUbo     = device->createBuffer(1, sizeof(Light));
        setLight({});
    }

    void DeferredShading::setSceneData(const SceneData& data)
    {
        sceneData = data;
        sceneDataUbo->uploadToGPU(&sceneData, 1);
    }

    void DeferredShading::setLight(const std::vector<Light>& lightInput)
    {
        light = lightInput;
        if (light.size() > 0)
            LightUbo->uploadToGPU(light.data(), 1);
    }

    void DeferredShading::setInputTextures(Vulcant::VulcantImage& colorInput, Vulcant::VulcantImage& depthInput, Vulcant::VulcantImage& normalInput)
    {
        color  = &colorInput;
        depth  = &depthInput;
        normal = &normalInput;

        glm::ivec2 resolution = color->getResolution();
        result                = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT);

        color->setUsage(Vulcant::VulcantImageUsage::SampleOnly);
        depth->setUsage(Vulcant::VulcantImageUsage::SampleOnly);
        normal->setUsage(Vulcant::VulcantImageUsage::SampleOnly);
        result->setUsage(Vulcant::VulcantImageUsage::WriteAndRead);
        set = device->createSet(
          {
            { color->asResource(), depth->asResource(), normal->asResource() },
            { result->asResource() },
            { sceneDataUbo->asResource() },
            { LightUbo->asResource() }
        },
          *shader);
    }

    void DeferredShading::record(Vulcant::VulcantComputeCommand& cmd)
    {
        glm::ivec2 resolution = color->getResolution();
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / 16.0f), (uint32_t)ceil(resolution.y / 16.0f), 1);

        cmd.add(groupCount, *set, *shader);
    }

    Vulcant::VulcantImage& DeferredShading::getTexture() const
    {
        return *result;
    }
}
