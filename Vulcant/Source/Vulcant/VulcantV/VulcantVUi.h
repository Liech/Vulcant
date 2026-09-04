#pragma once

#include "Library/Vulcant/Interface/VulcantUi.h"
#include "Library/Vulcant/Wrapper/Helper/VulcanUi.h"
#include <memory>

namespace Vulcant::VulcantV
{
    class VulcantVUi : public VulcantUi
    {
      public:
        VulcantVUi(Wrapper::VulcanDevice& device, Wrapper::VulcanPool& pool, Wrapper::Window& window);
        virtual ~VulcantVUi();

        virtual void newFrame() override;
        virtual void record(VulcantGraphicCommand& cmd, VulcantImage& targetImage) override;

      private:
        std::unique_ptr<Wrapper::VulcanUi> ui;
    };
}
