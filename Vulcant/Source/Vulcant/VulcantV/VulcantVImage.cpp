#include "VulcantVImage.h"

#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanResource.h"
#include "Library/Vulcant/VulcantV/VulcantVResource.h"
#include <stdexcept>

namespace Vulcant::VulcantV
{
    VulcantVImage::VulcantVImage(uint32_t width, uint32_t height, uint32_t depth, VulcantImageFormat format, Wrapper::VulcanPool& pool, Wrapper::VulcanDevice& device)
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
        else if (format == VulcantImageFormat::D32_SFLOAT)
            formatC = VK_FORMAT_D32_SFLOAT;
        else
            throw std::runtime_error("Unkown Image Format");
        img = std::make_unique<Wrapper::VulcanImage>(width, height,depth, formatC, pool, device);
    }

    VulcantVImage::VulcantVImage()
    {

    }

    VulcantVImage::~VulcantVImage() {}

    std::unique_ptr<VulcantResource> VulcantVImage::asResource() const
    {
        auto result = std::make_unique<VulcantVResource>();
        result->res = img->asResource();
        return std::move(result);
    }

    void VulcantVImage::saveRenderedImage(const std::string& path)
    {
        img->saveRenderedImage(path);
    }

    glm::ivec2 VulcantVImage::getResolution() const
    {
        return glm::ivec2(img->getWidth(), img->getHeight());
    }

    void VulcantVImage::setUsage(const Vulcant::VulcantImageUsage& usage)
    {
        img->setUsage(usage);
    }
}