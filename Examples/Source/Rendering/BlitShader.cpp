#include "BlitShader.h"
#include <stdexcept>
#include <string_view>

#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantComputeCommand.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"
#include "ShaderLibrary/Blit.h"

namespace Vulcant::Rendering
{
    BlitShader::BlitShader() {}

    BlitShader::~BlitShader() {}

    void BlitShader::setDevice(Vulcant::VulcantDevice& deviceInput)
    {
        if (device)
            throw std::runtime_error("Device already set");
        device = &deviceInput;

        const auto* slang      = Blit_spirv;
        auto        slang_size = Blit_spirv_sizeInBytes;
        auto        spirv      = std::vector<uint32_t>(slang, slang + slang_size / sizeof(uint32_t));

        shader = device->createShader(spirv);
    }

    void BlitShader::setInputTextures(Vulcant::VulcantImage& input_, std::shared_ptr<Vulcant::VulcantResource> screen)
    {
        if (!device)
            throw std::runtime_error("Device not set");

        input = &input_;

        input->setUsage(Vulcant::VulcantImageUsage::SampleOnly);

        set = device->createSet({ { screen }, { input->asResource() } }, *shader);

        glm::ivec2 resolution = input->getResolution();
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / 16.0f), (uint32_t)ceil(resolution.y / 16.0f), 1);
        cmd = device->createComputeCommand();
        cmd->startRecord();
        cmd->addBarrier(*input, Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd->add(groupCount, *set, *shader);
        cmd->addBarrier(*input, Vulcant::VulcantResourceLayout::General);
        cmd->endRecord();
    }

    void BlitShader::render()
    {
        if (!input)
            throw std::runtime_error("Device not set");
        cmd->runAsync();
    }
}