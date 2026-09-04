#pragma once

#include "VulcantInputValue.h"
#include <functional>
#include <glm/glm.hpp>

namespace Vulcant
{
    class VulcantInput
    {
      public:
        VulcantInput()          = default;
        virtual ~VulcantInput() = default;

        virtual void       setCallback(std::function<void(const VulcantInputValue&)> newInput) = 0;
        virtual glm::dvec2 getMousePosition() const                                            = 0;
        virtual void       setMousePosition(const glm::dvec2&) const                           = 0;
        virtual bool       isPressed(const VulcantInputValue&) const                           = 0;
    };
}