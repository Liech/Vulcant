#pragma once

#include "Library/Vulcant/Interface/VulcantUi.h"
#include <memory>

namespace Vulcant::VulcantG
{
    class VulcantGUi : public Vulcant::VulcantUi
    {
      public:
        VulcantGUi();
        virtual ~VulcantGUi();

        virtual void newFrame() override;
        virtual void record(VulcantGraphicCommand& cmd, VulcantImage& targetImage) override;
    };
}
