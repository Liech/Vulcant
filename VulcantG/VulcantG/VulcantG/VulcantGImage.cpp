#include "VulcantGImage.h"

#include "VulcantG/Wrapper/VulcanImage.h"
#include "VulcantG/Wrapper/VulcanResource.h"
#include "VulcantGResource.h"
#include <stdexcept>

namespace Vulcant::VulcantG
{
    VulcantGImage::VulcantGImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat format, Wrapper::VulcanDevice& device)
    {
        godot::RenderingDevice::DataFormat formatC;
        if (format == VulcantImageFormat::R32G32B32A32_SFLOAT)
            formatC = godot::RenderingDevice::DataFormat::DATA_FORMAT_R32G32B32A32_SFLOAT;
        else if (format == VulcantImageFormat::R16G16B16A16_SFLOAT)
            formatC = godot::RenderingDevice::DataFormat::DATA_FORMAT_R16G16B16A16_SFLOAT;
        else if (format == VulcantImageFormat::R8G8B8A8_UNORM)
            formatC = godot::RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM;
        else if (format == VulcantImageFormat::R32_SFLOAT)
            formatC = godot::RenderingDevice::DataFormat::DATA_FORMAT_R32_SFLOAT;
        else if (format == VulcantImageFormat::D32_SFLOAT)
            formatC = godot::RenderingDevice::DataFormat::DATA_FORMAT_D32_SFLOAT;
        else
            throw std::runtime_error("Unkown Image Format");
        img = std::make_unique<Wrapper::VulcanImage>(width, height,depth, formatC, device);
    }

    VulcantGImage::VulcantGImage()
    {

    }

    VulcantGImage::~VulcantGImage() {}

    std::unique_ptr<VulcantResource> VulcantGImage::asResource() const
    {
        auto result = std::make_unique<VulcantGResource>();
        result->res = img->asResource();
        return std::move(result);
    }

    void VulcantGImage::saveRenderedImage(const std::string& path)
    {
        img->saveRenderedImage(path);
    }

    glm::ivec2 VulcantGImage::getResolution() const
    {
        return glm::ivec2(img->getWidth(), img->getHeight());
    }

    void VulcantGImage::setUsage(const Vulcant::VulcantImageUsage& usage)
    {
        img->setUsage(usage);
    }
}