#include "VulcanImage.h"
#include <cassert>
#include <godot_cpp/classes/rd_sampler_state.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/texture2drd.hpp>
#include <iostream>
#include <lodepng.h>

#include "VulcanDevice.h"

namespace Vulcant::VulcantG::Wrapper
{
    VulcanImage::VulcanImage(uint32_t w, uint32_t h, uint32_t d, godot::RenderingDevice::DataFormat fmt, VulcanDevice& dev)
      : width(w)
      , height(h)
      , depth(d)
      , format(fmt)
      , device(dev)
      , ownsImage(true)
    {
        createImage();
    }

    VulcanImage::VulcanImage(godot::RID existingRid, VulcanDevice& dev)
      : device(dev)
      , imageRid(existingRid)
      , ownsImage(false)
    {
        using namespace godot;
        RenderingDevice& rd = device.getDevice();

        if (imageRid.is_valid())
        {
            Ref<RDTextureFormat> tf = rd.texture_get_format(imageRid);

            if (tf.is_valid())
            {
                width  = tf->get_width();
                height = tf->get_height();
                depth  = tf->get_depth();
                format = tf->get_format();
            }
        }
        else
            throw std::runtime_error("Invalid Image");
    }

    VulcanImage::~VulcanImage()
    {
        if (samplerRid.is_valid())
        {
            device.getDevice().free_rid(samplerRid);
        }
        if (imageRid.is_valid() && ownsImage)
        {
            device.getDevice().free_rid(imageRid);
        }
    }

    VulcanResource VulcanImage::asResource() const
    {
        if (usage == Vulcant::VulcantImageUsage::SampleOnly)
        {
            return { godot::RenderingDevice::UniformType::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, imageRid, samplerRid };
        }
        else
        {
            return { godot::RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE, imageRid };
        }
    }

    uint32_t VulcanImage::getWidth() const
    {
        return width;
    }

    uint32_t VulcanImage::getHeight() const
    {
        return height;
    }

    godot::RID VulcanImage::getRid() const
    {
        return imageRid;
    }

    void VulcanImage::createImage()
    {
        using namespace godot;
        RenderingDevice& rd = device.getDevice();

        Ref<RDTextureFormat> tf;
        tf.instantiate();
        tf->set_texture_type((depth > 1) ? RenderingDevice::TEXTURE_TYPE_3D : RenderingDevice::TEXTURE_TYPE_2D);
        tf->set_format(format); // e.g., DATA_FORMAT_R32G32B32A32_SFLOAT
        tf->set_width(width);
        tf->set_height(height);
        tf->set_depth(depth);
        tf->set_array_layers(1);
        tf->set_mipmaps(1);

        tf->set_usage_bits(RenderingDevice::TEXTURE_USAGE_STORAGE_BIT | RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_CAN_COPY_FROM_BIT |
                           RenderingDevice::TEXTURE_USAGE_CAN_UPDATE_BIT);
        Ref<RDTextureView> tv;
        tv.instantiate();

        imageRid = rd.texture_create(tf, tv);

        if (!imageRid.is_valid())
        {
            std::cout << "CRITICAL: Texture creation failed! Check format support for Intel UHD." << std::endl;
        }
    }

    godot::Ref<godot::Texture2DRD> VulcanImage::getAsDisplayTexture()
    {
        assert(depth == 1);
        if (!device.isGlobal())
        {
            std::cout << "Device not global. Image might not show up" << std::endl;
        }
        if (!imageRid.is_valid())
        {
            std::cout << "Image RID is not valid" << std::endl;
        }
        godot::Ref<godot::Texture2DRD> tex;
        tex.instantiate();
        tex->set_texture_rd_rid(imageRid);
        return tex;
    }

    void VulcanImage::saveRenderedImage(const std::string& path)
    {
        assert(depth == 1);
        godot::PackedByteArray data = device.getDevice().texture_get_data(imageRid, 0);

        const float* float_data = reinterpret_cast<const float*>(data.ptr());

        std::vector<unsigned char> image_8bit;
        image_8bit.reserve(width * height * 4);

        size_t total_elements = width * height * 4;
        for (size_t i = 0; i < total_elements; ++i)
        {
            float value = std::max(0.0f, std::min(1.0f, float_data[i]));
            image_8bit.push_back(static_cast<unsigned char>(value * 255.0f));
        }

        unsigned error = lodepng_encode32_file(path.c_str(), image_8bit.data(), width, height);

        if (error)
        {
            printf("LodePNG Error %u: %s\n", error, lodepng_error_text(error));
        }
    }

    void VulcanImage::setUsage(const Vulcant::VulcantImageUsage& inputUsage)
    {
        usage = inputUsage;
        if (!samplerRid.is_valid() && usage == Vulcant::VulcantImageUsage::SampleOnly)
        {
            createSampler();
        }
    }

    void VulcanImage::createSampler()
    {
        using namespace godot;
        RenderingDevice& rd = device.getDevice();

        // Equivalent to VkSamplerCreateInfo
        Ref<RDSamplerState> sampler_state;
        sampler_state.instantiate();
        sampler_state->set_mag_filter(RenderingDevice::SAMPLER_FILTER_LINEAR);
        sampler_state->set_min_filter(RenderingDevice::SAMPLER_FILTER_LINEAR);
        sampler_state->set_repeat_u(RenderingDevice::SAMPLER_REPEAT_MODE_REPEAT);
        sampler_state->set_repeat_v(RenderingDevice::SAMPLER_REPEAT_MODE_REPEAT);
        sampler_state->set_repeat_w(RenderingDevice::SAMPLER_REPEAT_MODE_REPEAT);

        samplerRid = rd.sampler_create(sampler_state);
    }
}