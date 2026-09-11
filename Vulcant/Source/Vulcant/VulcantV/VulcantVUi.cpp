#include "VulcantVUi.h"
#include "VulcantVGraphicCommand.h"
#include "VulcantVImage.h"
#include "VulcantVWindow.h"
#include "Vulcant/Wrapper/Graphic/VulcanGraphicCommand.h"
#include "Vulcant/Wrapper/VulcanImage.h"
#include "Vulcant/Wrapper/Window.h"

namespace Vulcant::VulcantV
{
    VulcantVUi::VulcantVUi(Wrapper::VulcanDevice& device, Wrapper::VulcanPool& pool, Wrapper::Window& window)
    {
        ui = std::make_unique<Wrapper::VulcanUi>(device, pool, window);
    }

    VulcantVUi::~VulcantVUi() {}

    void VulcantVUi::newFrame()
    {
        ui->newFrame();
    }

    void VulcantVUi::record(VulcantGraphicCommand& cmd, VulcantImage& targetImage)
    {
        auto& vcmd = static_cast<VulcantVGraphicCommand&>(cmd);
        auto& vimg = static_cast<VulcantVImage&>(targetImage);
        ui->render(*vcmd.cmd, *vimg.img);
    }
}
