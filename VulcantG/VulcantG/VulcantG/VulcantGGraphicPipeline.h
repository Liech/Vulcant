#pragma once

#include "Vulcant/Interface/VulcantGraphicPipeline.h"
#include <memory>
#include <vector>

namespace Vulcant::VulcantG
{
    class VulcantGDevice;
    class VulcantGShader;
    class VulcantGImage;

    namespace Wrapper
    {
        class VulcanGraphicPipeline;
    }

    class VulcantGGraphicPipeline : public VulcantGraphicPipeline
    {
      public:
        VulcantGGraphicPipeline(Vulcant::VulcantG::VulcantGDevice&  device,
                                const std::vector<VulcantGShader*>& shader,
                                const std::vector<VulcantGImage*>&  color,
                                VulcantGImage*                      depth   = nullptr,
                                VulcantGImage*                      stencil = nullptr);

        virtual ~VulcantGGraphicPipeline();

        std::unique_ptr<VulcantG::Wrapper::VulcanGraphicPipeline> pipe = nullptr;
    };
}
