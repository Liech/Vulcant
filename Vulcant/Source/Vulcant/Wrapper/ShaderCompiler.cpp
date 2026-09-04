#include "ShaderCompiler.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <shaderc/shaderc.hpp>
#include <spirv-reflect/spirv_reflect.h>
#include <stdexcept>

namespace Vulcant::Wrapper
{
    std::vector<uint32_t> ShaderCompiler::compile(const std::string& source)
    {
        try
        {
            shaderc::Compiler       compiler;
            shaderc::CompileOptions options;

            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

#ifdef _DEBUG
            options.SetOptimizationLevel(shaderc_optimization_level_zero);
            options.SetGenerateDebugInfo();
#else
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

            shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.c_str(),
                                                                             source.size(),
                                                                             shaderc_compute_shader,
                                                                             "compute_shader.comp", // File name used for error reporting
                                                                             "main",
                                                                             options);

            if (result.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                std::cerr << "Shader compilation FAILED: " << result.GetErrorMessage() << std::endl;
                throw std::runtime_error("GLSL to SPIR-V compilation failed: " + result.GetErrorMessage());
            }

            std::vector<uint32_t> spirv_data = { result.cbegin(), result.cend() };
            // std::cout << "Successfully compiled GLSL to SPIR-V!" << std::endl;
            // std::cout << "SPIR-V size: " << spirv_data.size() * sizeof(uint32_t) << " bytes." << std::endl;

            return spirv_data;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return {};
        }
    }
    std::vector<std::vector<ShaderBindingDefinition>> ShaderCompiler::getLayout(const std::vector<uint32_t>& spirv)
    {
        SpvReflectShaderModule module;
        SpvReflectResult       result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            return {};
        }

        uint32_t set_count = 0;
        spvReflectEnumerateDescriptorSets(&module, &set_count, nullptr);

        std::vector<SpvReflectDescriptorSet*> sets(set_count);
        spvReflectEnumerateDescriptorSets(&module, &set_count, sets.data());

        uint32_t max_set_index = 0;
        for (uint32_t i_set = 0; i_set < set_count; ++i_set)
        {
            max_set_index = std::max(max_set_index, sets[i_set]->set);
        }

        if (set_count == 0)
        {
            spvReflectDestroyShaderModule(&module);
            return {};
        }

        std::vector<std::vector<ShaderBindingDefinition>> layouts(max_set_index + 1);

        for (uint32_t i_set = 0; i_set < set_count; ++i_set)
        {
            auto*    refl_set       = sets[i_set];
            uint32_t target_set_idx = refl_set->set;

            uint32_t max_binding = 0;
            for (uint32_t i_b = 0; i_b < refl_set->binding_count; ++i_b)
            {
                max_binding = std::max(max_binding, refl_set->bindings[i_b]->binding);
            }

            if (refl_set->binding_count > 0)
            {
                layouts[target_set_idx].resize(max_binding + 1);
            }

            for (uint32_t i_b = 0; i_b < refl_set->binding_count; ++i_b)
            {
                auto& refl_binding = *(refl_set->bindings[i_b]);

                ShaderBindingDefinition def{};
                def.type = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

                def.count = 1;
                for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim)
                {
                    def.count *= refl_binding.array.dims[i_dim];
                }

                if (def.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                {
                    def.layout = VK_IMAGE_LAYOUT_GENERAL;
                }
                else
                {
                    def.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }

                layouts[target_set_idx][refl_binding.binding] = def;
            }
        }

        spvReflectDestroyShaderModule(&module);
        return layouts;
    }

    VkShaderStageFlagBits ShaderCompiler::getShaderStage(const std::vector<uint32_t>& spirv)
    {
        SpvReflectShaderModule module;
        SpvReflectResult       result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            throw std::runtime_error("Shader-Reflektion fehlgeschlagen!");
        }

        VkShaderStageFlagBits stage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

        spvReflectDestroyShaderModule(&module);
        return stage;
    }

    VkFormat ShaderCompiler::mapSpvFormatToVk(SpvReflectInterfaceVariable* var)
    {
        if (var->numeric.scalar.width == 32)
        {
            switch (var->numeric.vector.component_count)
            {
                case 1:
                    return VK_FORMAT_R32_SFLOAT;
                case 2:
                    return VK_FORMAT_R32G32_SFLOAT;
                case 3:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case 4:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
            }
        }

        if (var->numeric.scalar.width == 32 && var->numeric.scalar.signedness == 1)
        {
            switch (var->numeric.vector.component_count)
            {
                case 1:
                    return VK_FORMAT_R32_SINT;
                case 2:
                    return VK_FORMAT_R32G32_SINT;
                case 3:
                    return VK_FORMAT_R32G32B32_SINT;
                case 4:
                    return VK_FORMAT_R32G32B32A32_SINT;
            }
        }

        throw std::runtime_error("Nicht unterstütztes Vertex-Format im Shader: " + std::string(var->name));
    }

    uint32_t ShaderCompiler::getSizeOfVkFormat(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R32_SFLOAT:
                return 4;
            case VK_FORMAT_R32G32_SFLOAT:
                return 8;
            case VK_FORMAT_R32G32B32_SFLOAT:
                return 12;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 16;
            case VK_FORMAT_R32_SINT:
                return 4;
            case VK_FORMAT_R32G32_SINT:
                return 8;
            case VK_FORMAT_R32G32B32_SINT:
                return 12;
            case VK_FORMAT_R32G32B32A32_SINT:
                return 16;
            default:
                throw std::runtime_error("Format-Größe für dieses VkFormat noch nicht implementiert.");
        }
    }

    std::vector<VertexAttribute> ShaderCompiler::getVertexInputLayout(const std::vector<uint32_t>& spirv)
    {
        SpvReflectShaderModule module; 
        spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);

        uint32_t var_count = 0;
        spvReflectEnumerateInputVariables(&module, &var_count, nullptr);
        std::vector<SpvReflectInterfaceVariable*> input_vars(var_count);
        spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());

        std::vector<VertexAttribute> attributes;
        for (auto* var : input_vars)
        {
            if (var->built_in == -1)
            {
                bool isInstance = false;
                if (var->name != nullptr && std::string(var->name).find("instance") != std::string::npos)
                {
                    isInstance = true;
                }

                auto f = mapSpvFormatToVk(var);
                attributes.push_back({ var->location,
                                       f,
                                       getSizeOfVkFormat(f), 
                                       isInstance ? InputRate::Instance : InputRate::Vertex });
            }
        }
        std::sort(attributes.begin(), attributes.end(), [](auto& a, auto& b) { return a.location < b.location; });

        spvReflectDestroyShaderModule(&module);
        return attributes;
    }
}