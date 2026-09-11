#pragma once

#include "Vulcant/Interface/VulcantDevice.h"

namespace Vuclant
{
    class VulcantComputeCommand;
    class VulcantGraphicPipeline;
}

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanPool;
    class VulcanInstance;
}

namespace Vulcant::VulcantV
{
    class VulcantVDevice : public VulcantDevice
    {
      public:
        VulcantVDevice(const std::vector<std::string>& extensions = {}, bool debug = true);
        virtual ~VulcantVDevice();

        virtual std::unique_ptr<VulcantBuffer>          createBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly) override;
        virtual std::unique_ptr<VulcantBuffer>          createUniform(size_t numberOfElements, size_t elementSize) override;
        virtual std::unique_ptr<VulcantBuffer>          createVertexBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly) override;
        virtual std::unique_ptr<VulcantShader>          createShader(const std::string& glsl) override;
        virtual std::unique_ptr<VulcantShader>          createShader(const std::vector<uint32_t>& spirv) override;
        virtual std::unique_ptr<VulcantImage>           createImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat = VulcantImageFormat::R32G32B32A32_SFLOAT) override;
        virtual std::unique_ptr<VulcantComputeCommand>  createComputeCommand() override;
        virtual std::unique_ptr<VulcantGraphicCommand>  createGraphicCommand() override;
        virtual std::unique_ptr<VulcantGraphicPipeline> createVulcanGraphicPipeline(const std::vector<VulcantShader*>& shader,
                                                                                    const std::vector<VulcantImage*>&  color,
                                                                                    VulcantImage*                      depth   = nullptr,
                                                                                    VulcantImage*                      stencil = nullptr) override;
        virtual std::unique_ptr<VulcantSet>             createSet(const std::vector<std::vector<std::shared_ptr<VulcantResource>>>& buffer, VulcantShader& shader) override;
        virtual std::unique_ptr<VulcantWindow>          createWindow(const glm::ivec2& resolution, const std::string& title) override;
        virtual std::unique_ptr<VulcantUi>              createUi(VulcantWindow& window) override;
        virtual VulcantComputeCommand&                  getDefaultCommand() override;

        Vulcant::Wrapper::VulcanDevice& __getDevice() const;
        Vulcant::Wrapper::VulcanPool&   __getPool() const;

      private:
        std::unique_ptr<Vulcant::Wrapper::VulcanInstance> instance;
        std::unique_ptr<Vulcant::Wrapper::VulcanDevice>   device;
        std::unique_ptr<Vulcant::Wrapper::VulcanPool>     pool;
        std::unique_ptr<Vulcant::VulcantComputeCommand>   defaultCmd = nullptr;
    };
}