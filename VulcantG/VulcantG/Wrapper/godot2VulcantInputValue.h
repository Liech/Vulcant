#pragma once

#include "Library/Vulcant/Interface/VulcantInputValue.h"
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>

namespace Vulcant::VulcantG::Wrapper
{
    VulcantInputValue godotKeyboard2Vulcant(const godot::Ref<godot::InputEventKey>& p_event);
    VulcantInputValue godotMouse2Vulcant(const godot::Ref<godot::InputEventMouseButton>& p_event);
}