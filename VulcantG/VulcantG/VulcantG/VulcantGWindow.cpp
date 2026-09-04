#include "VulcantGWindow.h"

#include "VulcantG/VulcantG/VulcantGImage.h"
#include "VulcantG/VulcantG/VulcantGInput.h"
#include "VulcantG/Wrapper/VulcanCompositor.h"
#include "VulcantG/Wrapper/VulcanNode.h"
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/window.hpp>

namespace Vulcant::VulcantG
{
    VulcantGWindow::VulcantGWindow(const glm::ivec2& resolution, const std::string& title, godot::Node* rootInput)
    {
        assert(rootInput != nullptr);
        root = rootInput;
        createNode();
        node->get_window()->set_size(godot::Vector2i(resolution.x, resolution.y));
        node->get_window()->set_title(title.c_str());
        compositor = VulcantG::Wrapper::VulcanCompositor::installCompositorEffect(rootInput);
        input      = std::make_unique<VulcantG::VulcantGInput>(*this);
        input->setMouseCallback([this](const glm::dvec2& pos) { godot::Input::get_singleton()->warp_mouse(godot::Vector2(pos.x, pos.y)); });
        lastTime = std::chrono::steady_clock::now();
    }

    VulcantGWindow::~VulcantGWindow()
    {
        if (node != nullptr)
        {
            node->dispose();
            compositor->uninstallCompositorEffect(root);
            if (node->is_inside_tree())
            {
                node->queue_free();
            }
            else
            {
                memdelete(node);
            }
            node = nullptr;
        }
    }

    void VulcantGWindow::start(const std::function<void(double delta)>& onLogicInput, const std::function<void()>& onRender, const std::function<void(const glm::ivec2& resolution)>& onResize)
    {
        onLogic = onLogicInput;
        compositor->start(onRender, onResize);
    }

    void VulcantGWindow::blitImage(Vulcant::VulcantImage& color)
    {
        compositor->blitImage(color);
    }

    bool VulcantGWindow::isClosed() const
    {
        return !open;
    }

    void VulcantGWindow::tick()
    {
        input->tick();

        auto                          currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed     = currentTime - lastTime;
        double                        deltaTime   = elapsed.count();
        lastTime                                  = currentTime;
        double clampedDelta                       = std::min(deltaTime, 0.1);

        onLogic(deltaTime);
    }

    glm::ivec2 VulcantGWindow::getResolution() const
    {
        if (!node)
            return glm::ivec2(0, 0);
        auto result = node->get_window()->get_size();
        return glm::ivec2(result.x, result.y);
    }

    void VulcantGWindow::createNode()
    {
        node = memnew(Vulcant::VulcantG::Wrapper::VulcanNode);
        root->add_child(node);
        node->set_owner(root);
        node->set_name("Vulcant");
        node->initialize(
          [this]() // exit_tree
          {
              if (node)
              {
                  node->dispose();
                  compositor->uninstallCompositorEffect(root);
                  node = nullptr;
                  open = false;
              }
          },
          [this]() { tick(); },
          [this](Vulcant::VulcantInputValue v) { input->set(v, true); },
          [this](Vulcant::VulcantInputValue v) { input->set(v, false); });
        open = true;
    }

    Vulcant::VulcantInput& VulcantGWindow::getInput()
    {
        return *input;
    }

    glm::dvec2 VulcantGWindow::getMousePosition() const
    {
        godot::Vector2 pos = node->get_viewport()->get_mouse_position();
        return glm::dvec2(pos.x, getResolution().y-pos.y);
    }
}