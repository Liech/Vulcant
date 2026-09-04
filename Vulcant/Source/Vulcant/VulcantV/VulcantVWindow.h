#pragma once

#include "Library/Vulcant/Interface/VulcantInputValue.h"
#include "Library/Vulcant/Interface/VulcantWindow.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Vulcant
{
    class VulcantImage;
    class VulcantDevice;
    class VulcantInput;
    namespace Wrapper
    {
        class Window;
    }
}

namespace Vulcant::VulcantV
{
    class VulcantVInput;

    class VulcantVWindow : public VulcantWindow
    {
      public:
        VulcantVWindow(Vulcant::VulcantDevice& device, const glm::ivec2& resolution, const std::string& title);
        virtual ~VulcantVWindow();

        virtual void start(
          const std::function<void(double)>&                       onLogic  = [](double delta) {},
          const std::function<void()>&                             onRender = []() {},
          const std::function<void(const glm::ivec2& resolution)>& onResize = [](const glm::ivec2& resolution) {});

        virtual bool                   isClosed() const;
        virtual void                   tick();
        virtual void                   blitImage(Vulcant::VulcantImage&);
        virtual glm::ivec2             getResolution() const;
        virtual Vulcant::VulcantInput& getInput();

        void       setMousePosition(const glm::dvec2& pos);
        glm::dvec2 getMousePosition() const;
        bool       isPressed(const VulcantInputValue&) const;

        Wrapper::Window& getWrapper();

      private:
        std::unique_ptr<Wrapper::Window> window;
        std::unique_ptr<VulcantVInput>   input;
    };
}