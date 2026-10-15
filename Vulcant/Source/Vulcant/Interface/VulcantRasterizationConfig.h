#pragma once

#include "VulcantCullMode.h"
#include "VulcantFrontFace.h"
#include "VulcantPolygonMode.h"

namespace Vulcant
{
    class VulcantRasterizationConfig
    {
      public:
        VulcantRasterizationConfig();
        virtual ~VulcantRasterizationConfig();

        VulcantCullMode getCullMode() const;
        void            setCullMode(VulcantCullMode mode);

        VulcantFrontFace getFrontFace() const;
        void             setFrontFace(VulcantFrontFace face);

        VulcantPolygonMode getPolygonMode() const;
        void               setPolygonMode(VulcantPolygonMode mode);

      private:
        VulcantCullMode    cullMode    = VulcantCullMode::None;
        VulcantFrontFace   frontFace   = VulcantFrontFace::Clockwise;
        VulcantPolygonMode polygonMode = VulcantPolygonMode::Fill;
    };
}
