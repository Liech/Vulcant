#pragma once

#include "VulcantG/VulcantG/VulcantGImage.h"
#include "Library/Vulcant/Interface/VulcantInputValue.h"
#include <functional>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>

namespace Vulcant
{
    class VulcantImage;
    class VulcantDevice;
}

namespace Vulcant::VulcantG
{
    class VulcantGDevice;
}

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanImage;
    class VulcanDevice;

    // anchor to get updates about the scene
    class VulcanNode : public godot::Node
    {
        GDCLASS(VulcanNode, godot::Node)

      public:
        VulcanNode();
        virtual ~VulcanNode();

        void initialize(std::function<void()>                           onTreeExitCallback,
                        std::function<void()>                           onTick,
                        std::function<void(Vulcant::VulcantInputValue)> keyDown,
                        std::function<void(Vulcant::VulcantInputValue)> keyUp);
        void dispose();

        virtual void _exit_tree() override;
        virtual void _unhandled_input(const godot::Ref<godot::InputEvent>& p_event) override;
        virtual void _physics_process(double p_delta);
      private:
        std::function<void()>                           onTreeExitCallback = []() {};
        std::function<void()>                           onTick = []() {};
        std::function<void(Vulcant::VulcantInputValue)> keyDown            = [](Vulcant::VulcantInputValue) {};
        std::function<void(Vulcant::VulcantInputValue)> keyUp              = [](Vulcant::VulcantInputValue) {};

      protected:
        static void _bind_methods();
    };
}