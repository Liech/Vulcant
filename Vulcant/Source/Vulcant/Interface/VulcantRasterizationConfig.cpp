#include "VulcantRasterizationConfig.h"

namespace Vulcant
{
    VulcantRasterizationConfig::VulcantRasterizationConfig()
    {
    }

    VulcantRasterizationConfig::~VulcantRasterizationConfig()
    {
    }

    VulcantCullMode VulcantRasterizationConfig::getCullMode() const
    {
        return cullMode;
    }

    void VulcantRasterizationConfig::setCullMode(VulcantCullMode mode)
    {
        cullMode = mode;
    }

    VulcantFrontFace VulcantRasterizationConfig::getFrontFace() const
    {
        return frontFace;
    }

    void VulcantRasterizationConfig::setFrontFace(VulcantFrontFace face)
    {
        frontFace = face;
    }

    VulcantPolygonMode VulcantRasterizationConfig::getPolygonMode() const
    {
        return polygonMode;
    }

    void VulcantRasterizationConfig::setPolygonMode(VulcantPolygonMode mode)
    {
        polygonMode = mode;
    }
}

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>

TEST_CASE("VulcantRasterizationConfig/Defaults")
{
    Vulcant::VulcantRasterizationConfig config;
    REQUIRE(config.getCullMode() == Vulcant::VulcantCullMode::None);
    REQUIRE(config.getFrontFace() == Vulcant::VulcantFrontFace::Clockwise);
    REQUIRE(config.getPolygonMode() == Vulcant::VulcantPolygonMode::Fill);
}

TEST_CASE("VulcantRasterizationConfig/Setters")
{
    Vulcant::VulcantRasterizationConfig config;
    config.setCullMode(Vulcant::VulcantCullMode::Back);
    config.setFrontFace(Vulcant::VulcantFrontFace::CounterClockwise);
    config.setPolygonMode(Vulcant::VulcantPolygonMode::Line);

    REQUIRE(config.getCullMode() == Vulcant::VulcantCullMode::Back);
    REQUIRE(config.getFrontFace() == Vulcant::VulcantFrontFace::CounterClockwise);
    REQUIRE(config.getPolygonMode() == Vulcant::VulcantPolygonMode::Line);
}
#endif
