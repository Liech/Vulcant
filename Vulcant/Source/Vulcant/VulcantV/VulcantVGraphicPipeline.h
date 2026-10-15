#pragma once

#include <memory>
#include <vector>
#include "Vulcant/Interface/VulcantGraphicPipeline.h"
#include "Vulcant/Interface/VulcantRasterizationConfig.h"
#include "Vulcant/Wrapper/Graphic/VulcanGraphicPipeline.h"

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanShader;
    class VulcanImage;
}

namespace Vulcant::VulcantV
{
    class VulcantVGraphicPipeline : public VulcantGraphicPipeline
    {
      public:
        VulcantVGraphicPipeline(Vulcant::Wrapper::VulcanDevice&                     device,
                                const std::vector<Vulcant::Wrapper::VulcanShader*>& shader,
                                const std::vector<Vulcant::Wrapper::VulcanImage*>&  color,
                                Vulcant::Wrapper::VulcanImage*                      depth        = nullptr,
                                Vulcant::Wrapper::VulcanImage*                      stencil      = nullptr,
                                const VulcantRasterizationConfig&                   rasterConfig = VulcantRasterizationConfig());

        virtual ~VulcantVGraphicPipeline();

        std::unique_ptr<Vulcant::Wrapper::VulcanGraphicPipeline> pipe = nullptr;
    };
}
