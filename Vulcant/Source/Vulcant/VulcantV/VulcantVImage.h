#pragma once

#include "Library/Vulcant/Interface/VulcantImage.h"
#include "Library/Vulcant/Interface/VulcantImageFormat.h"
#include <memory>

namespace Vulcant::Wrapper
{
    class VulcanPool;
    class VulcanDevice;
    class VulcanImage;
}

namespace Vulcant::VulcantV
{
    class VulcantVImage : public VulcantImage
    {
      public:
        VulcantVImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat format, Wrapper::VulcanPool& pool, Wrapper::VulcanDevice& device);
        VulcantVImage();
        virtual ~VulcantVImage();

        virtual glm::ivec2                       getResolution() const override;
        virtual std::unique_ptr<VulcantResource> asResource() const override;
        virtual void                             saveRenderedImage(const std::string& path) override;
        virtual void                             setUsage(const Vulcant::VulcantImageUsage& usage) override;

        std::unique_ptr<Wrapper::VulcanImage> img = nullptr;
    };
}