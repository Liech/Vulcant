#pragma once

#include "Vulcant/Interface/VulcantUi.h"
#include <memory>

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanDevice;
        class VulcanUi;
    }
    class VulcantGWindow;

    class VulcantGUi : public Vulcant::VulcantUi
    {
      public:
        VulcantGUi(Wrapper::VulcanDevice& device, VulcantGWindow* window);
        VulcantGUi();
        virtual ~VulcantGUi();

        virtual void newFrame() override;
        virtual void record(VulcantGraphicCommand& cmd, VulcantImage& targetImage) override;

      private:
        std::unique_ptr<Wrapper::VulcanUi> ui;
    };
}
