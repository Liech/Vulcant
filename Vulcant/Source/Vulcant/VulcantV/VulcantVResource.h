#pragma once

#include "Vulcant/Wrapper/VulcanResource.h"
#include "Vulcant/Interface/VulcantResource.h"

namespace Vulcant::VulcantV
{
  class VulcantVResource : public VulcantResource
    {
      public:
        Wrapper::VulcanResource res;
  };
}