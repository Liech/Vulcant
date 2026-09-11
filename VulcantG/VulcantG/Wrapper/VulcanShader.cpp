#include "VulcanShader.h"

#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>

#include "VulcanDevice.h"
#include "Vulcant/Wrapper/ShaderCompiler.h"

namespace Vulcant::VulcantG::Wrapper
{
    VulcanShader::VulcanShader(const std::string& source, VulcanDevice& deviceInput, godot::RenderingDevice::ShaderStage stage)
      : device(deviceInput)
    {
        layouts  = Vulcant::Wrapper::ShaderCompiler::getLayout(Vulcant::Wrapper::ShaderCompiler::compile(source));
        shader   = compile(source, device, stage);
        pipeline = device.getDevice().compute_pipeline_create(shader);
    }

    VulcanShader::VulcanShader(const godot::String& source, VulcanDevice& deviceInput, godot::RenderingDevice::ShaderStage stage)
      : device(deviceInput)
    {
        shader   = compile(source, device, stage);
        pipeline = device.getDevice().compute_pipeline_create(shader);
    }

    VulcanShader::VulcanShader(const std::vector<uint32_t>& spriv, VulcanDevice& deviceInput, godot::RenderingDevice::ShaderStage stage)
      : device(deviceInput)
    {
        layouts = Vulcant::Wrapper::ShaderCompiler::getLayout(spriv);
        godot::PackedByteArray bytecode;
        bytecode.resize(spriv.size() * sizeof(uint32_t));
        if (!spriv.empty())
        {
            memcpy(bytecode.ptrw(), spriv.data(), bytecode.size());
        }
        godot::Ref<godot::RDShaderSPIRV> rd_spirv;
        rd_spirv.instantiate();
        rd_spirv->set_stage_bytecode(stage, bytecode);
        auto binary = device.getDevice().shader_compile_binary_from_spirv(rd_spirv);
        shader      = device.getDevice().shader_create_from_bytecode(binary);

        if (!shader.is_valid())
        {
            throw std::runtime_error("Failed to create shader from Slang-generated SPIR-V.");
        }

        pipeline = device.getDevice().compute_pipeline_create(shader);
    }

    godot::RID VulcanShader::getShader() const
    {
        return shader;
    }
    godot::RID VulcanShader::getPipeline() const
    {
        return pipeline;
    }

    void VulcanShader::bind(int64_t compute_list)
    {
        device.getDevice().compute_list_bind_compute_pipeline(compute_list, pipeline);
    }

    godot::RID VulcanShader::compile(const std::string& source, VulcanDevice& device, godot::RenderingDevice::ShaderStage stage)
    {
        godot::String str = godot::String(source.c_str());
        return compile(str, device, stage);
    }

    godot::RID VulcanShader::compile(const godot::String& source, VulcanDevice& device, godot::RenderingDevice::ShaderStage stage)
    {
        godot::Ref<godot::RDShaderSource> shader_source;
        shader_source.instantiate();
        shader_source->set_stage_source(stage, source);
        auto spirv_code = device.getDevice().shader_compile_spirv_from_source(shader_source);
        auto binary     = device.getDevice().shader_compile_binary_from_spirv(spirv_code);
        auto result     = device.getDevice().shader_create_from_bytecode(binary);

        if (!result.is_valid())
        {
            throw std::runtime_error("Failed to create shader from SPIR-V.");
        }

        return result;
    }

    const std::vector<std::vector<Vulcant::Wrapper::ShaderBindingDefinition>>& VulcanShader::getLayout() const
    {
        return layouts;
    }
}