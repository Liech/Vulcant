#pragma once

#include "Vulcant/Interface/VulcantWindow.h"
#include <godot_cpp/classes/ref.hpp>
#include <map>
#include <memory>
#include <string>
#include <chrono>

namespace Vulcant
{
    class VulcantImage;
}

namespace godot
{
    class Node;
}

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanCompositor;
        class VulcanNode;
    }
    class VulcantGInput;

    class VulcantGWindow : public VulcantWindow
    {
      public:
        VulcantGWindow(const glm::ivec2& resolution, const std::string& title, godot::Node* root);
        virtual ~VulcantGWindow();

        virtual void start(
          const std::function<void(double delta)>&                 onLogic  = [](double delta) {},
          const std::function<void()>&                             onRender = []() {},
          const std::function<void(const glm::ivec2& resolution)>& onResize = [](const glm::ivec2& resolution) {});

        virtual bool                   isClosed() const;
        virtual void                   tick();
        virtual void                   blitImage(Vulcant::VulcantImage&);
        virtual glm::ivec2             getResolution() const;
        virtual Vulcant::VulcantInput& getInput();
        glm::dvec2                     getMousePosition() const;

      private:
        void createNode();

        Vulcant::VulcantG::Wrapper::VulcanNode*                  node;
        godot::Ref<Vulcant::VulcantG::Wrapper::VulcanCompositor> compositor;
        std::function<void(double delta)>                        onLogic = [](double delta) {};
        std::unique_ptr<Vulcant::VulcantG::VulcantGInput>        input;

        godot::Node* root = nullptr;
        bool                                  open = false;
        std::chrono::steady_clock::time_point lastTime;
    };
}