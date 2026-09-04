#include "VulcantVShader.h"

#include "Library/Vulcant/Wrapper/ShaderCompiler.h"
#include "Library/Vulcant/Wrapper/VulcanShader.h"

namespace Vulcant::VulcantV
{
    VulcantVShader::VulcantVShader(const std::string& source, Wrapper::VulcanDevice& device)
    {
        auto spirv = Wrapper::ShaderCompiler::compile(source);
        shader     = std::make_unique<Wrapper::VulcanShader>(spirv, device);
    }

    VulcantVShader::VulcantVShader(const std::vector<uint32_t>& spirv, Wrapper::VulcanDevice& device)
    {
        shader     = std::make_unique<Wrapper::VulcanShader>(spirv, device);
    }

    VulcantVShader::~VulcantVShader() {}
}