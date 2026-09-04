#include "Example.h"

#include "SphereImage.h"
#include "MandelbrotImage.h"
#include "CubeCaster.h"
#include "CubeCasterFreecam.h"

namespace Vulcant::Examples
{
    std::vector<std::shared_ptr<Example>> Example::getAll()
    {
        std::vector<std::shared_ptr<Example>> result;
        result.push_back(std::make_shared<MandelbrotImage>());
        result.push_back(std::make_shared<SphereImage>());
        result.push_back(std::make_shared<CubeCaster>());
        result.push_back(std::make_shared<CubeCasterFreecam>());
        return result;
    }
}