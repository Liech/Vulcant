#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "VulcantImageUsage.h"

namespace Vulcant
{
    class VulcantResource;

    class VulcantImage
    {
      public:
        VulcantImage()          = default;
        virtual ~VulcantImage() = default;

        virtual glm::ivec2                       getResolution() const                      = 0;
        virtual std::unique_ptr<VulcantResource> asResource() const                         = 0;
        virtual void                             saveRenderedImage(const std::string& path) = 0;
        virtual void                             setUsage(const Vulcant::VulcantImageUsage& usage) = 0;
    };
}