#pragma once

#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantImageFormat.h"
#include <memory>

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanDevice;
        class VulcanImage;
    }

    class VulcantGImage : public VulcantImage
    {
      public:
        VulcantGImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat format, Wrapper::VulcanDevice& device);
        VulcantGImage();
        virtual ~VulcantGImage();

        virtual glm::ivec2                       getResolution() const override;
        virtual std::unique_ptr<VulcantResource> asResource() const override;
        virtual void                             saveRenderedImage(const std::string& path) override;
        virtual void                             setUsage(const Vulcant::VulcantImageUsage& usage) override;


        std::unique_ptr<Wrapper::VulcanImage> img = nullptr;
    };
}