#include "VulcantGGraphicPipeline.h"

#include "VulcantG/Wrapper/VulcanGraphicPipeline.h"
#include "VulcantG/Wrapper/VulcanShader.h"
#include "VulcantG/Wrapper/VulcanImage.h"
#include "VulcantGDevice.h"
#include "VulcantGShader.h"
#include "VulcantGImage.h"

namespace Vulcant::VulcantG
{
    VulcantGGraphicPipeline::VulcantGGraphicPipeline(Vulcant::VulcantG::VulcantGDevice&  device,
                                                     const std::vector<VulcantGShader*>& shader,
                                                     const std::vector<VulcantGImage*>&  color,
                                                     VulcantGImage*                      depth,
                                                     VulcantGImage*                      stencil)
    {
        std::vector<Wrapper::VulcanShader*> wShader;
        for (const auto& x : shader)
            wShader.push_back(x->shader.get());
        std::vector<Wrapper::VulcanImage*> wColor;
        for (const auto& x : color)
            wColor.push_back(x->img.get());

        pipe = std::make_unique<Wrapper::VulcanGraphicPipeline>(device.__getDevice(), wShader, wColor, depth->img.get(), stencil->img.get());
    }

    VulcantGGraphicPipeline::~VulcantGGraphicPipeline() {}
}
