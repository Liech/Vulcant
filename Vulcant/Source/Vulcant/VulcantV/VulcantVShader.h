#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Vulcant/Interface/VulcantShader.h"

namespace Vulcant::Wrapper
{
    class VulcanShader;
    class VulcanDevice;
}

namespace Vulcant::VulcantV
{
    class VulcantVShader : public VulcantShader
    {
      public:
        VulcantVShader(const std::string& source, Wrapper::VulcanDevice& device);
        VulcantVShader(const std::vector<uint32_t>& spirv, Wrapper::VulcanDevice& device);
        virtual ~VulcantVShader();

        std::unique_ptr<Wrapper::VulcanShader> shader;
    };
}