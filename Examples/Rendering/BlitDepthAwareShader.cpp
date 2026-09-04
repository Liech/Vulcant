#include "BlitDepthAwareShader.h"
#include <stdexcept>
#include <string_view>

#include "Library/Vulcant/Interface/VulcantBuffer.h"
#include "Library/Vulcant/Interface/VulcantComputeCommand.h"
#include "Library/Vulcant/Interface/VulcantDevice.h"
#include "Library/Vulcant/Interface/VulcantImage.h"
#include "Library/Vulcant/Interface/VulcantResource.h"
#include "Library/Vulcant/Interface/VulcantSet.h"
#include "Library/Vulcant/Interface/VulcantShader.h"
#include "ShaderLibrary/Blit_DepthAware.h"

namespace Vulcant::Rendering
{
    BlitDepthAwareShader::BlitDepthAwareShader() {}

    BlitDepthAwareShader::~BlitDepthAwareShader() {}

    void BlitDepthAwareShader::setDevice(Vulcant::VulcantDevice& deviceInput, Vulcant::VulcantComputeCommand& cmdInput)
    {
        if (device)
            throw std::runtime_error("Device already set");
        device = &deviceInput;
        cmd    = &cmdInput;

        const auto* slang      = Blit_DepthAware_spirv;
        auto        slang_size = Blit_DepthAware_spirv_sizeInBytes;
        auto        spirv      = std::vector<uint32_t>(slang, slang + slang_size / sizeof(uint32_t));

        shader = device->createShader(spirv);
    }

    void BlitDepthAwareShader::setInputTextures(Vulcant::VulcantImage&                    input_,
                                                Vulcant::VulcantImage&                    input_depth_,
                                                std::shared_ptr<Vulcant::VulcantResource> screen,
                                                std::shared_ptr<Vulcant::VulcantResource> screen_depth)
    {
        if (!device)
            throw std::runtime_error("Device not set");

        input       = &input_;
        input_depth = &input_depth_;

        input_depth->setUsage(Vulcant::VulcantImageUsage::SampleOnly);
        input->setUsage(Vulcant::VulcantImageUsage::SampleOnly);

        set = device->createSet({ { screen }, { input->asResource() }, { screen_depth }, { input_depth->asResource() } }, *shader);
    }

    void BlitDepthAwareShader::render()
    {
        if (!input)
            throw std::runtime_error("Device not set");

        glm::ivec2 resolution = input->getResolution();
        glm::ivec3 groupCount = glm::ivec3((uint32_t)ceil(resolution.x / 16.0f), (uint32_t)ceil(resolution.y / 16.0f), 1);

        cmd->startRecord();
        cmd->addBarrier(*input, Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd->addBarrier(*input_depth, Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd->add(groupCount, *set, *shader);
        cmd->addBarrier(*input, Vulcant::VulcantResourceLayout::General);
        cmd->addBarrier(*input_depth, Vulcant::VulcantResourceLayout::General);
        cmd->endRecord();
    }
}