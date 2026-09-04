#pragma once

namespace Vulcant
{
    enum class VulcantImageFormat
    {
        R32G32B32A32_SFLOAT, // VK_FORMAT_R32G32B32A32_SFLOAT
        R16G16B16A16_SFLOAT, // VK_FORMAT_R16G16B16A16_SFLOAT
        R8G8B8A8_UNORM,      // VK_FORMAT_R8G8B8A8_UNORM
        R32_SFLOAT,          // VK_FORMAT_R32_SFLOAT
        D32_SFLOAT,          // VK_FORMAT_D32_SFLOAT

        Unkown
    };
}