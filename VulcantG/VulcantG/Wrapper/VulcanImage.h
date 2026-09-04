#pragma once

#include "Library/Vulcant/Interface/VulcantImageUsage.h"
#include "VulcanResource.h"
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/texture2drd.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <string>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;

    class VulcanImage
    {
      public:
        VulcanImage(uint32_t width, uint32_t height, uint32_t depth, godot::RenderingDevice::DataFormat format, VulcanDevice& device);
        VulcanImage(godot::RID, VulcanDevice& dev);
        virtual ~VulcanImage();

        godot::Ref<godot::Texture2DRD> getAsDisplayTexture();
        void                           setUsage(const Vulcant::VulcantImageUsage& inputUsage);

        void saveRenderedImage(const std::string& path);
        // godot::RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM

        VulcanResource asResource() const;

        godot::RID getRid() const;
        uint32_t   getWidth() const;
        uint32_t   getHeight() const;

      private:
        void createImage();
        void createSampler();

        VulcanDevice&                      device;
        uint32_t                           width, height, depth;
        godot::RenderingDevice::DataFormat format;
        godot::RID                         imageRid;
        godot::RID                         samplerRid;
        VulcantImageUsage                  usage;
        bool                               ownsImage = true;
    };
}