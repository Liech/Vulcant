#pragma once

#include <memory>

namespace Vulcant
{
    class VulcantGraphicCommand;
    class VulcantImage;

    class VulcantUi
    {
      public:
        virtual void newFrame()                                                     = 0;
        virtual void record(VulcantGraphicCommand& cmd, VulcantImage& targetImage) = 0;
    };
}
