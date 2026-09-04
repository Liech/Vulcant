#pragma once

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/texture2drd.hpp>

namespace Vulcant::VulcantG
{
    class VulcantGDevice;
}
namespace Vulcant
{
    class VulcantWindow;
}
namespace Vulcant::Examples
{
    class Example;
}

namespace godot
{
    class ExampleRunner : public godot::Node3D
    {
        GDCLASS(ExampleRunner, godot::Node3D)

      protected:
        static void _bind_methods();

      public:
        ExampleRunner();
        ~ExampleRunner();

        virtual void _exit_tree() override;

      private:
        TypedArray<godot::String> listExamples();
        void                      runExample(int index, const Vector2i& resolution);
        void                      setRoot(godot::Node*);

        std::shared_ptr<Vulcant::Examples::Example>        currentExample = nullptr;
        std::unique_ptr<Vulcant::VulcantG::VulcantGDevice> device         = nullptr;
    };
}