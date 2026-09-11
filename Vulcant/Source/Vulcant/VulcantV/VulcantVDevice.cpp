#include "VulcantVDevice.h"

#include "Vulcant/Wrapper/VulcanBuffer.h"
#include "Vulcant/Wrapper/VulcanDevice.h"
#include "Vulcant/Wrapper/VulcanInstance.h"
#include "Vulcant/Wrapper/VulcanPool.h"
#include "VulcantVBuffer.h"
#include "VulcantVComputeCommand.h"
#include "VulcantVGraphicCommand.h"
#include "VulcantVGraphicPipeline.h"
#include "VulcantVImage.h"
#include "VulcantVResource.h"
#include "VulcantVSet.h"
#include "VulcantVShader.h"
#include "VulcantVUi.h"
#include "VulcantVWindow.h"

namespace Vulcant::VulcantV
{
    VulcantVDevice::VulcantVDevice(const std::vector<std::string>& extensions, bool debug)
    {
        instance = std::make_unique<Wrapper::VulcanInstance>(extensions, debug);
        device   = std::make_unique<Wrapper::VulcanDevice>(*instance);
        pool     = std::make_unique<Wrapper::VulcanPool>(*device);
    }

    VulcantVDevice::~VulcantVDevice() {}

    VulcantComputeCommand& VulcantVDevice::getDefaultCommand()
    {
        if (defaultCmd == nullptr)
        {
            defaultCmd = createComputeCommand();
        }
        return *defaultCmd;
    }

    std::unique_ptr<VulcantBuffer> VulcantVDevice::createBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly)
    {
        return std::make_unique<VulcantVBuffer>(numberOfElements, elementSize, *device, VulcantBufferType::Storage, gpuOnly);
    }

    std::unique_ptr<VulcantBuffer> VulcantVDevice::createVertexBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly)
    {
        return std::make_unique<VulcantVBuffer>(numberOfElements, elementSize, *device, VulcantBufferType::Vertex, gpuOnly);
    }

    std::unique_ptr<VulcantBuffer> VulcantVDevice::createUniform(size_t numberOfElements, size_t elementSize)
    {
        return std::make_unique<VulcantVBuffer>(numberOfElements, elementSize, *device, VulcantBufferType::Uniform, false);
    }

    std::unique_ptr<VulcantComputeCommand> VulcantVDevice::createComputeCommand()
    {
        return std::make_unique<VulcantVComputeCommand>(*pool, *device);
    }

    std::unique_ptr<VulcantGraphicCommand> VulcantVDevice::createGraphicCommand()
    {
        return std::make_unique<VulcantVGraphicCommand>(*pool, *device);
    }

    std::unique_ptr<VulcantShader> VulcantVDevice::createShader(const std::string& shader)
    {
        return std::make_unique<VulcantVShader>(shader, *device);
    }

    std::unique_ptr<VulcantShader> VulcantVDevice::createShader(const std::vector<uint32_t>& spirv)
    {
        return std::make_unique<VulcantVShader>(spirv, *device);
    }

    std::unique_ptr<VulcantImage> VulcantVDevice::createImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat format)
    {
        return std::make_unique<VulcantVImage>(width, height, depth, format, *pool, *device);
    }

    std::unique_ptr<VulcantSet> VulcantVDevice::createSet(const std::vector<std::vector<std::shared_ptr<VulcantResource>>>& buffer, VulcantShader& shader)
    {
        std::vector<std::vector<Wrapper::VulcanResource>> bufferConverted;
        for (const auto& lay : buffer)
        {
            bufferConverted.push_back({});
            auto& sub = bufferConverted.back();
            for (const auto& r : lay)
            {
                auto& resRef = static_cast<VulcantVResource*>(r.get())->res;
                sub.push_back(resRef);
            }
        }
        auto& s = *((VulcantVShader&)shader).shader;
        return std::make_unique<VulcantVSet>(bufferConverted, s, *pool);
    }

    std::unique_ptr<VulcantWindow> VulcantVDevice::createWindow(const glm::ivec2& resolution, const std::string& title)
    {
        return std::make_unique<VulcantVWindow>(*this, resolution, title);
    }

    std::unique_ptr<VulcantGraphicPipeline> VulcantVDevice::createVulcanGraphicPipeline(const std::vector<VulcantShader*>& shader,
                                                                                         const std::vector<VulcantImage*>&  color,
                                                                                         VulcantImage*                      depth,
                                                                                         VulcantImage*                      stencil)
    {
        std::vector<Vulcant::Wrapper::VulcanShader*> shaderConverted;
        for (auto* s : shader)
        {
            shaderConverted.push_back(static_cast<VulcantVShader*>(s)->shader.get());
        }

        std::vector<Vulcant::Wrapper::VulcanImage*> colorConverted;
        for (auto* c : color)
        {
            colorConverted.push_back(static_cast<VulcantVImage*>(c)->img.get());
        }

        Vulcant::Wrapper::VulcanImage* depthConverted = nullptr;
        if (depth)
        {
            depthConverted = static_cast<VulcantVImage*>(depth)->img.get();
        }

        Vulcant::Wrapper::VulcanImage* stencilConverted = nullptr;
        if (stencil)
        {
            stencilConverted = static_cast<VulcantVImage*>(stencil)->img.get();
        }

        return std::make_unique<VulcantVGraphicPipeline>(*device, shaderConverted, colorConverted, depthConverted, stencilConverted);
    }

    std::unique_ptr<VulcantUi> VulcantVDevice::createUi(VulcantWindow& window)
    {
        auto& vwin = static_cast<VulcantVWindow&>(window);
        return std::make_unique<VulcantVUi>(*device, *pool, vwin.getWrapper());
    }

    Vulcant::Wrapper::VulcanDevice& VulcantVDevice::__getDevice() const
    {
        return *device;
    }

    Vulcant::Wrapper::VulcanPool& VulcantVDevice::__getPool() const
    {
        return *pool;
    }
}