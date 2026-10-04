#pragma once

#include <memory>
#include <vector>
#include "Vulcant/Wrapper/Graphic/VulcanGraphicPipeline.h"
#include "Vulcant/Interface/VulcantGraphicPipeline.h"

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
                                Vulcant::Wrapper::VulcanImage*                      depth   = nullptr,
                                Vulcant::Wrapper::VulcanImage*                      stencil = nullptr,
                                VkPrimitiveTopology                                 topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        virtual ~VulcantVGraphicPipeline();

        std::unique_ptr<Vulcant::Wrapper::VulcanGraphicPipeline> pipe = nullptr;
    };
}
