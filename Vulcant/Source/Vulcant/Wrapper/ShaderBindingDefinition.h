#pragma once

#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;

    struct ShaderBindingDefinition
    {
        uint32_t         count;
        VkDescriptorType type;
        VkImageLayout    layout;
    };
}