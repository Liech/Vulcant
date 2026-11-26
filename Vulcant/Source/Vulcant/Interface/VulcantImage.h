#pragma once

#include "VulcantImageUsage.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Vulcant
{
    class VulcantResource;

    class VulcantImage
    {
      public:
        VulcantImage();
        virtual ~VulcantImage();

        virtual glm::ivec2                       getResolution() const                                     = 0;
        virtual std::unique_ptr<VulcantResource> asResource() const                                        = 0;
        virtual void                             saveRenderedImage(const std::string& path)                = 0;
        virtual void                             setUsage(const Vulcant::VulcantImageUsage& usage)         = 0;
        virtual std::vector<uint8_t>             readPixelsRgba8()                                         = 0;
        virtual void                             uploadPixelsRgba8(const std::vector<uint8_t>& rgbaPixels) = 0;
    };
}