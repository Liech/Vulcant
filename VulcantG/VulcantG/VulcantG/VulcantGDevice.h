#pragma once

#include "Library/Vulcant/Interface/VulcantDevice.h"

namespace godot
{
    class Node;
}

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanDevice;
        class VulcanPool;
        class VulcanInstance;
    }

    class VulcantGDevice : public VulcantDevice
    {
      public:
        VulcantGDevice(bool local = false, godot::Node* root = nullptr);
        virtual ~VulcantGDevice();

        virtual std::unique_ptr<VulcantBuffer>         createBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly) override;
        virtual std::unique_ptr<VulcantBuffer>         createUniform(size_t numberOfElements, size_t elementSize) override;
        virtual std::unique_ptr<VulcantBuffer>         createVertexBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly) override;
        virtual std::unique_ptr<VulcantShader>         createShader(const std::string& shader) override;
        virtual std::unique_ptr<VulcantShader>         createShader(const std::vector<uint32_t>& spirv) override;
        virtual std::unique_ptr<VulcantImage>          createImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat = VulcantImageFormat::R32G32B32A32_SFLOAT) override;
        virtual std::unique_ptr<VulcantComputeCommand> createComputeCommand() override;
        virtual std::unique_ptr<VulcantGraphicCommand> createGraphicCommand() override;
        virtual std::unique_ptr<VulcantSet>            createSet(const std::vector<std::vector<std::shared_ptr<VulcantResource>>>& buffer, VulcantShader& shader) override;
        virtual std::unique_ptr<VulcantWindow>         createWindow(const glm::ivec2& resolution, const std::string& title) override;
        virtual VulcantComputeCommand&                 getDefaultCommand() override;
        virtual std::unique_ptr<VulcantUi>             createUi(VulcantWindow& window) override;

        virtual std::unique_ptr<VulcantGraphicPipeline> createVulcanGraphicPipeline(const std::vector<VulcantShader*>& shader,
                                                                                    const std::vector<VulcantImage*>&  color,
                                                                                    VulcantImage*                      depth   = nullptr,
                                                                                    VulcantImage*                      stencil = nullptr) override;

        Wrapper::VulcanDevice& __getDevice();

      private:
        std::unique_ptr<Wrapper::VulcanDevice> device;
        godot::Node*                           root       = nullptr;
        std::unique_ptr<VulcantComputeCommand> defaultCmd = nullptr;
    };
}