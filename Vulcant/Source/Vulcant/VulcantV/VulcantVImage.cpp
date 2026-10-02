#include "VulcantVImage.h"

#include "Vulcant/Wrapper/VulcanImage.h"
#include "Vulcant/Wrapper/VulcanResource.h"
#include "Vulcant/VulcantV/VulcantVResource.h"
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
        else if (format == VulcantImageFormat::R32_UINT)
            formatC = VK_FORMAT_R32_UINT;
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

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>
#include "VulcantVDevice.h"
#include "VulcantVResource.h"

//TEST_CASE("VulcantVImage Creation and Usage Modification", "[VulcantVImage]")
//{
//    Vulcant::VulcantV::VulcantVDevice device({}, false);
//
//    auto img = device.createImage(256, 128, 1, Vulcant::VulcantImageFormat::R8G8B8A8_UNORM);
//    REQUIRE(img != nullptr);
//    REQUIRE(img->getResolution() == glm::ivec2(256, 128));
//
//    Vulcant::VulcantImageUsage usage;
//    usage.storage = true;
//    usage.sampled = true;
//    img->setUsage(usage);
//
//    auto resource = img->asResource();
//    REQUIRE(resource != nullptr);
//}

TEST_CASE("VulcantVImage Various Formats Creation", "[VulcantVImage]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    auto img32f = device.createImage(64, 64, 1, Vulcant::VulcantImageFormat::R32_SFLOAT);
    REQUIRE(img32f != nullptr);

    auto imgDepth = device.createImage(64, 64, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);
    REQUIRE(imgDepth != nullptr);

    auto img16f = device.createImage(64, 64, 1, Vulcant::VulcantImageFormat::R16G16B16A16_SFLOAT);
    REQUIRE(img16f != nullptr);

    auto img32uint = device.createImage(64, 64, 1, Vulcant::VulcantImageFormat::R32_UINT);
    REQUIRE(img32uint != nullptr);
}
#endif