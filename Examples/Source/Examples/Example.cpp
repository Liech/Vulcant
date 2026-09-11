#include "Example.h"

#include "SphereImage.h"
#include "MandelbrotImage.h"
#include "CubeCaster.h"
#include "CubeCasterFreecam.h"
#include "UiExample.h"
#include "GraphicPipelineTriangle.h"
#include "GraphicPipelineCube.h"

namespace Vulcant::Examples
{
    std::vector<std::shared_ptr<Example>> Example::getAll()
    {
        std::vector<std::shared_ptr<Example>> result;
        result.push_back(std::make_shared<MandelbrotImage>());
        result.push_back(std::make_shared<SphereImage>());
        result.push_back(std::make_shared<CubeCaster>());
        result.push_back(std::make_shared<CubeCasterFreecam>());
        result.push_back(std::make_shared<UiExample>());
        result.push_back(std::make_shared<GraphicPipelineTriangle>());
        result.push_back(std::make_shared<GraphicPipelineCube>());
        return result;
    }
}