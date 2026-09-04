#pragma once

#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace Vulcant::VulcantG::Wrapper
{
    struct VulcanResource
    {
        godot::RenderingDevice::UniformType type;
        godot::RID                          rid;
        godot::RID                          sampler;
    };
}