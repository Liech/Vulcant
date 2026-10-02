#include "VulcantVShader.h"

#include "Vulcant/Wrapper/ShaderCompiler.h"
#include "Vulcant/Wrapper/VulcanShader.h"

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

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>
#include "VulcantVDevice.h"

TEST_CASE("VulcantVShader Compilation from GLSL Source", "[VulcantVShader]")
{
    Vulcant::VulcantV::VulcantVDevice device({}, false);

    std::string glslComputeShader = R"(
        #version 450
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(std430, set = 0, binding = 0) buffer DataBuffer {
            uint data[];
        };
        void main() {
            uint id = gl_GlobalInvocationID.x;
            data[id] = data[id] * 2;
        }
    )";

    auto shader = device.createShader(glslComputeShader);
    REQUIRE(shader != nullptr);
}
#endif