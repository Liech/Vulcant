#pragma once

#include "ShaderBindingDefinition.h"
#include <string>
#include <vector>

struct SpvReflectInterfaceVariable;

namespace Vulcant::Wrapper
{
    struct ShaderBindingDefinition;

    enum class InputRate
    {
        Vertex,
        Instance
    };

    struct VertexAttribute
    {
        uint32_t location;
        VkFormat format;
        uint32_t size;
        InputRate rate;
    };

    class ShaderCompiler
    {
      public:
        static std::vector<uint32_t>                             compile(const std::string& source);
        static std::vector<std::vector<ShaderBindingDefinition>> getLayout(const std::vector<uint32_t>& spirv);
        static VkShaderStageFlagBits                             getShaderStage(const std::vector<uint32_t>& spirv);
        static std::vector<VertexAttribute>                      getVertexInputLayout(const std::vector<uint32_t>& spirv);

      private:
        static VkFormat mapSpvFormatToVk(SpvReflectInterfaceVariable* var);
        static uint32_t getSizeOfVkFormat(VkFormat format);
    };
}