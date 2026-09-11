#include "ExampleRunner.h"

#include "Examples/Example.h"
#include "VulcantG/VulcantG/VulcantGDevice.h"
#include <glm/gtc/type_ptr.hpp>
#include <godot_cpp/classes/Engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>

namespace godot
{
    // --- Binding and Initialization ---
    void ExampleRunner::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("listExamples"), &ExampleRunner::listExamples);
        ClassDB::bind_method(D_METHOD("setRoot", "root"), &ExampleRunner::setRoot);
        ClassDB::bind_method(D_METHOD("runExample", "index", "resolution"), &ExampleRunner::runExample);
    }

    ExampleRunner::ExampleRunner()
    {
    }

    ExampleRunner::~ExampleRunner() {}

    void ExampleRunner::_exit_tree()
    {
        currentExample = nullptr;
    }

    void ExampleRunner::setRoot(godot::Node* root)
    {
        device = std::make_unique<Vulcant::VulcantG::VulcantGDevice>(false, root);
    }

    void ExampleRunner::runExample(int index, const Vector2i& resolution)
    {
        auto all              = Vulcant::Examples::Example::getAll();
        currentExample        = all[index];
        currentExample->createWindow(*device, glm::ivec2(resolution.x, resolution.y));
    }

    TypedArray<godot::String> ExampleRunner::listExamples()
    {
        auto                      all = Vulcant::Examples::Example::getAll();
        TypedArray<godot::String> result;
        for (const auto& x : all)
            result.push_back(x->getName().c_str());
        return result;
    }
}