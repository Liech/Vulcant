#include "VulcantGUi.h"

#include "VulcantG/Wrapper/VulcanUi.h"
#include "VulcantG/Wrapper/VulcanGraphicCommand.h"
#include "VulcantG/Wrapper/VulcanImage.h"
#include "VulcantG/VulcantG/VulcantGGraphicCommand.h"
#include "VulcantG/VulcantG/VulcantGImage.h"
#include "VulcantG/VulcantG/VulcantGWindow.h"

namespace Vulcant::VulcantG
{
    VulcantGUi::VulcantGUi(Wrapper::VulcanDevice& device, VulcantGWindow* window)
    {
        ui = std::make_unique<Wrapper::VulcanUi>(device, window);
    }

    VulcantGUi::VulcantGUi() {}

    VulcantGUi::~VulcantGUi() {}

    void VulcantGUi::newFrame()
    {
        if (ui)
        {
            ui->newFrame();
        }
    }

    void VulcantGUi::record(VulcantGraphicCommand& cmd, VulcantImage& targetImage)
    {
        if (ui)
        {
            auto& gcmd = static_cast<VulcantGGraphicCommand&>(cmd);
            auto& gimg = static_cast<VulcantGImage&>(targetImage);
            ui->render(*gcmd.cmd, *gimg.img);
        }
    }
}
