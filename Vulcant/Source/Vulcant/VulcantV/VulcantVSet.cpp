#include "VulcantVSet.h"

#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Wrapper/VulcanResource.h"
#include "Vulcant/Wrapper/VulcanSet.h"
#include "Vulcant/VulcantV/VulcantVResource.h"

namespace Vulcant::VulcantV
{
    VulcantVSet::VulcantVSet(const std::vector<std::vector<Wrapper::VulcanResource>>& buffer, Wrapper::VulcanShader& pipeline, Wrapper::VulcanPool& thread)
    {
        set = std::make_unique<Wrapper::VulcanSet>(buffer, pipeline, thread);
    }

    VulcantVSet::~VulcantVSet() {}

    void VulcantVSet::updateResource(size_t set_idx, size_t binding_idx, const VulcantResource& newResource)
    {
        auto& resRef = static_cast<const VulcantVResource&>(newResource).res;
        set->updateResource(set_idx, binding_idx, resRef);
    }
}

#ifdef ISTESTPROJECT
#include <catch2/catch_test_macros.hpp>
#include "VulcantVDevice.h"
#include "Vulcant/Interface/VulcantBuffer.h"
#include "VulcantVShader.h"

TEST_CASE("VulcantVSet Creation and Resource Update", "[VulcantVSet]")
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

    auto buf1 = device.createBuffer(10, sizeof(uint32_t), false);
    auto res1 = std::shared_ptr<Vulcant::VulcantResource>(buf1->asResource());

    std::vector<std::vector<std::shared_ptr<Vulcant::VulcantResource>>> resources = {
        { res1 }
    };

    auto set = device.createSet(resources, *shader);
    REQUIRE(set != nullptr);

    auto buf2 = device.createBuffer(10, sizeof(uint32_t), false);
    auto res2 = buf2->asResource();
    set->updateResource(0, 0, *res2);
}
#endif