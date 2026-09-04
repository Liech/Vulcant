#pragma once

#include "Library/Vulcant/Interface/VulcantShader.h"
#include <memory>
#include <string>
#include <vector>

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanShader;
        class VulcanDevice;
    }

    class VulcantGShader : public VulcantShader
    {
      public:
        VulcantGShader(const std::string& source, VulcantG::Wrapper::VulcanDevice& device);
        VulcantGShader(const std::vector<uint32_t>& spriv, VulcantG::Wrapper::VulcanDevice& device);
        virtual ~VulcantGShader();

        std::unique_ptr<VulcantG::Wrapper::VulcanShader> shader;
    };
}