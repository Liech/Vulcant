#include "VulcantVUi.h"
#include "VulcantVGraphicCommand.h"
#include "VulcantVImage.h"
#include "VulcantVWindow.h"
#include "Vulcant/Wrapper/Graphic/VulcanGraphicCommand.h"
#include "Vulcant/Wrapper/VulcanImage.h"
#include "Vulcant/Wrapper/Window.h"

namespace Vulcant::VulcantV
{
    VulcantVUi::VulcantVUi(Wrapper::VulcanDevice& device, Wrapper::VulcanPool& pool, Wrapper::Window& window, VulcantImageFormat format, bool clear)
    {
        VkFormat formatC;
        if (format == VulcantImageFormat::R16G16B16A16_SFLOAT)
            formatC = VK_FORMAT_R16G16B16A16_SFLOAT;
        else if (format == VulcantImageFormat::R32G32B32A32_SFLOAT)
            formatC = VK_FORMAT_R32G32B32A32_SFLOAT;
        else if (format == VulcantImageFormat::R8G8B8A8_UNORM)
            formatC = VK_FORMAT_R8G8B8A8_UNORM;
        else if (format == VulcantImageFormat::R32_SFLOAT)
            formatC = VK_FORMAT_R32_SFLOAT;
        else if (format == VulcantImageFormat::R32_UINT)
            formatC = VK_FORMAT_R32_UINT;
        else if (format == VulcantImageFormat::D32_SFLOAT)
            formatC = VK_FORMAT_D32_SFLOAT;
        else
            throw std::runtime_error("Unkown Image Format");
        ui = std::make_unique<Wrapper::VulcanUi>(device, pool, window, formatC, clear);
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
