#include "VulcanNode.h"
#include <stdexcept>

#include "Library/Vulcant/Interface/VulcantImage.h"
#include "VulcantG/VulcantG/VulcantGDevice.h"
#include "VulcantG/Wrapper/VulcanDevice.h"
#include "VulcantG/Wrapper/VulcanImage.h"
#include <godot_cpp/classes/compositor.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/world_environment.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include "godot2VulcantInputValue.h"

namespace Vulcant::VulcantG::Wrapper
{
    void VulcanNode::_bind_methods() {}

    VulcanNode::VulcanNode() {}

    VulcanNode::~VulcanNode() {}

    void VulcanNode::initialize(std::function<void()>                           onTreeExitCallbackInput,
                                std::function<void()>                           onTickInput,
                                std::function<void(Vulcant::VulcantInputValue)> keyDownInput,
                                std::function<void(Vulcant::VulcantInputValue)> keyUpInput)
    {
        onTreeExitCallback = onTreeExitCallbackInput;
        keyDown            = keyDownInput;
        keyUp              = keyUpInput;
        onTick             = onTickInput;
    }

    void VulcanNode::dispose()
    {
        onTreeExitCallback = []() {};
        keyDown            = [](Vulcant::VulcantInputValue) {};
        keyUp              = [](Vulcant::VulcantInputValue) {};
        onTick             = []() {};
    }

    void VulcanNode::_exit_tree()
    {
        onTreeExitCallback();
    }
    
    void VulcanNode::_physics_process(double p_delta)
    {
        onTick();
    }

    void VulcanNode::_unhandled_input(const godot::Ref<godot::InputEvent>& p_event)
    {
        godot::Ref<godot::InputEventKey> key_event = p_event;
        if (key_event.is_valid())
        {
            if (key_event->is_pressed() && !key_event->is_echo())
            {
                keyDown(godotKeyboard2Vulcant(key_event));
            }
            else if (key_event->is_released())
            {
                keyUp(godotKeyboard2Vulcant(key_event));
            }
            return;
        }

        godot::Ref<godot::InputEventMouseButton> mouse_event = p_event;
        if (mouse_event.is_valid())
        {
            if (mouse_event->is_pressed())
            {
                keyDown(godotMouse2Vulcant(mouse_event));
            }
            else if (mouse_event->is_released())
            {
                keyUp(godotMouse2Vulcant(mouse_event));
            }
        }
    }
}