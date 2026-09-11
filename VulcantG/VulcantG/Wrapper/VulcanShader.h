#pragma once

#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <string>
#include "Vulcant/Wrapper/ShaderBindingDefinition.h"

namespace godot
{
    class RenderingDevice;
}

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;

    class VulcanShader
    {
      public:
        VulcanShader(const std::string& source, VulcanDevice& device, godot::RenderingDevice::ShaderStage stage = godot::RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE);
        VulcanShader(const godot::String& source, VulcanDevice& device, godot::RenderingDevice::ShaderStage stage = godot::RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE);
        VulcanShader(const std::vector<uint32_t>& spriv, VulcanDevice& device, godot::RenderingDevice::ShaderStage stage = godot::RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE);

        static godot::RID compile(const std::string& source, VulcanDevice&, godot::RenderingDevice::ShaderStage = godot::RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE);
        static godot::RID compile(const godot::String& source, VulcanDevice&, godot::RenderingDevice::ShaderStage = godot::RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE);

        godot::RID getShader() const;
        godot::RID getPipeline() const;

        void bind(int64_t compute_list);

        const std::vector<std::vector<Vulcant::Wrapper::ShaderBindingDefinition>>& getLayout() const;

      private:
        std::vector<std::vector<Vulcant::Wrapper::ShaderBindingDefinition>> layouts;
        VulcanDevice& device;

        godot::RID shader;
        godot::RID pipeline;
    };
}