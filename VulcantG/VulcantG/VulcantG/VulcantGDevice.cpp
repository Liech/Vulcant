#include "VulcantGDevice.h"

#include "VulcantG/Wrapper/VulcanBuffer.h"
#include "VulcantG/Wrapper/VulcanDevice.h"
#include "VulcantGBuffer.h"
#include "VulcantGComputeCommand.h"
#include "VulcantGGraphicCommand.h"
#include "VulcantGGraphicPipeline.h"
#include "VulcantGImage.h"
#include "VulcantGResource.h"
#include "VulcantGSet.h"
#include "VulcantGShader.h"
#include "VulcantGUi.h"
#include "VulcantGWindow.h"

namespace Vulcant::VulcantG
{
    VulcantGDevice::VulcantGDevice(bool local, godot::Node* rootInput)
      : root(rootInput)
    {
        device = std::make_unique<Wrapper::VulcanDevice>(local);
    }

    VulcantGDevice::~VulcantGDevice() {}

    VulcantComputeCommand& VulcantGDevice::getDefaultCommand()
    {
        if (defaultCmd == nullptr)
            defaultCmd = createComputeCommand();
        return *defaultCmd;
    }

    std::unique_ptr<VulcantBuffer> VulcantGDevice::createBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly)
    {
        return std::make_unique<VulcantGBuffer>(numberOfElements, elementSize, *device, true, gpuOnly);
    }

    std::unique_ptr<VulcantBuffer> VulcantGDevice::createVertexBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly)
    {
        return std::make_unique<VulcantGBuffer>(numberOfElements, elementSize, *device, false, gpuOnly);
    }

    std::unique_ptr<VulcantBuffer> VulcantGDevice::createUniform(size_t numberOfElements, size_t elementSize)
    {
        return std::make_unique<VulcantGBuffer>(numberOfElements, elementSize, *device, false, false);
    }

    std::unique_ptr<VulcantComputeCommand> VulcantGDevice::createComputeCommand()
    {
        return std::make_unique<VulcantGComputeCommand>(*device);
    }

    std::unique_ptr<VulcantGraphicCommand> VulcantGDevice::createGraphicCommand()
    {
        return std::make_unique<VulcantGGraphicCommand>(*device);
    }

    std::unique_ptr<VulcantGraphicPipeline> VulcantGDevice::createVulcanGraphicPipeline(const std::vector<VulcantShader*>& shader,
                                                                                        const std::vector<VulcantImage*>&  color,
                                                                                        VulcantImage*                      depth,
                                                                                        VulcantImage*                      stencil)
    {
        std::vector<VulcantGImage*> gcolor;
        gcolor.reserve(color.size());
        for (auto* x : color)
            gcolor.push_back(static_cast<VulcantGImage*>(x));

        std::vector<VulcantGShader*> gshader;
        gshader.reserve(shader.size());
        for (auto* s : shader)
            gshader.push_back(static_cast<VulcantGShader*>(s));

        return std::make_unique<VulcantGGraphicPipeline>(*this, gshader, gcolor, static_cast<VulcantGImage*>(depth), static_cast<VulcantGImage*>(stencil));
    }

    std::unique_ptr<VulcantShader> VulcantGDevice::createShader(const std::string& shader)
    {
        return std::make_unique<VulcantGShader>(shader, *device);
    }

    std::unique_ptr<VulcantShader> VulcantGDevice::createShader(const std::vector<uint32_t>& spirv)
    {
        return std::make_unique<VulcantGShader>(spirv, *device);
    }

    std::unique_ptr<VulcantImage> VulcantGDevice::createImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat format)
    {
        return std::make_unique<VulcantGImage>(width, height, depth, format, *device);
    }

    std::unique_ptr<VulcantSet> VulcantGDevice::createSet(const std::vector<std::vector<std::shared_ptr<VulcantResource>>>& buffer, VulcantShader& shader)
    {
        std::vector<std::vector<Wrapper::VulcanResource>> bufferConverted;
        for (const auto& lay : buffer)
        {
            bufferConverted.push_back({});
            auto& sub = bufferConverted.back();
            for (const auto& r : lay)
            {
                auto& resRef = static_cast<VulcantGResource*>(r.get())->res;
                sub.push_back(resRef);
            }
        }
        auto& s = *((VulcantGShader&)shader).shader;
        return std::make_unique<VulcantGSet>(bufferConverted, s, *device);
    }

    std::unique_ptr<VulcantWindow> VulcantGDevice::createWindow(const glm::ivec2& resolution, const std::string& title)
    {
        return std::make_unique<VulcantGWindow>(resolution, title, root);
    }

    Wrapper::VulcanDevice& VulcantGDevice::__getDevice()
    {
        return *device;
    }

    std::unique_ptr<VulcantUi> VulcantGDevice::createUi(VulcantWindow& window)
    {
        return std::make_unique<VulcantGUi>();
    }
}