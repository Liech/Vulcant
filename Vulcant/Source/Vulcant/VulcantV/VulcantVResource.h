#pragma once

#include "Library/Vulcant/Wrapper/VulcanResource.h"
#include "Library/Vulcant/Interface/VulcantResource.h"

namespace Vulcant::VulcantV
{
  class VulcantVResource : public VulcantResource
    {
      public:
        Wrapper::VulcanResource res;
  };
}