#pragma once

#include <functional>
#include <glm/glm.hpp>

namespace Vulcant
{
    class VulcantImage;
    class VulcantInput;

    class VulcantWindow
    {
      public:
        VulcantWindow()          = default;
        virtual ~VulcantWindow() = default;

        virtual void start(
          const std::function<void(double delta)>&                 onLogic  = [](double delta) {},
          const std::function<void()>&                             onRender = []() {},
          const std::function<void(const glm::ivec2& resolution)>& onResize = [](const glm::ivec2& resolution) {}) = 0;

        virtual bool                   isClosed() const                  = 0;
        virtual void                   tick()                            = 0;
        virtual void                   blitImage(Vulcant::VulcantImage&) = 0;
        virtual glm::ivec2             getResolution() const             = 0;
        virtual Vulcant::VulcantInput& getInput()                        = 0;
    };
}