#include "VulcantGUi.h"

#include <stdexcept>

namespace Vulcant::VulcantG
{
    VulcantGUi::VulcantGUi() {}

    VulcantGUi::~VulcantGUi() {}

    void VulcantGUi::newFrame()
    {
        throw std::runtime_error("Not implemented!");
    }

    void VulcantGUi::record(VulcantGraphicCommand& cmd, VulcantImage& targetImage)
    {
        throw std::runtime_error("Not implemented!");
    }
}
