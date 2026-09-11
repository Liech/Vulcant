#include "VulcantVGraphicPipeline.h"

#include "Vulcant/Wrapper/Graphic/VulcanGraphicPipeline.h"

namespace Vulcant::VulcantV
{
    VulcantVGraphicPipeline::VulcantVGraphicPipeline(Vulcant::Wrapper::VulcanDevice& device, const std::vector<Vulcant::Wrapper::VulcanShader*>& shader, const std::vector<Vulcant::Wrapper::VulcanImage*>& color, Vulcant::Wrapper::VulcanImage* depth, Vulcant::Wrapper::VulcanImage* stencil)
    {
        pipe = std::make_unique<Vulcant::Wrapper::VulcanGraphicPipeline>(device, shader, color, depth, stencil);
    }

    VulcantVGraphicPipeline::~VulcantVGraphicPipeline() {}
}
