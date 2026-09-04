#include "VulcantGShader.h"

#include "VulcantG/Wrapper/VulcanShader.h"

namespace Vulcant::VulcantG
{
    VulcantGShader::VulcantGShader(const std::string& source, Wrapper::VulcanDevice& device)
    {
        shader = std::make_unique<Wrapper::VulcanShader>(source, device);
    }

    VulcantGShader::VulcantGShader(const std::vector<uint32_t>& spriv, Wrapper::VulcanDevice& device)
    {
        shader = std::make_unique<Wrapper::VulcanShader>(spriv, device);
    }

    VulcantGShader::~VulcantGShader() {}
}