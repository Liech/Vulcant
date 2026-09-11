#pragma once

#include "Vulcant/Interface/VulcantResource.h"
#include "VulcantG/Wrapper/VulcanResource.h"

namespace Vulcant::VulcantG
{
    class VulcantGResource : public VulcantResource
    {
      public:
        Wrapper::VulcanResource res;
    };
}