#pragma once

#include <functional>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    struct VulcanResource
    {
        VkDescriptorType                   type;
        VkDescriptorBufferInfo             bufferInfo          = {};
        VkDescriptorImageInfo              imageInfo           = {};
        VkImageLayout                      previousImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        std::function<void(VkImageLayout)> setLayout           = [](VkImageLayout) {};
    };
}