#pragma once

#include "Vulcant/Interface/VulcantInput.h"
#include <memory>
#include <vector>

namespace Vulcant::VulcantV
{
    class VulcantVWindow;

    class VulcantVInput : public VulcantInput
    {
      public:
        VulcantVInput(VulcantVWindow&);
        virtual ~VulcantVInput();

        virtual void       setCallback(std::function<void(const VulcantInputValue&)> newInput);
        virtual glm::dvec2 getMousePosition() const;
        virtual void       setMousePosition(const glm::dvec2&) const;
        virtual bool       isPressed(const VulcantInputValue&) const;

        void call(const VulcantInputValue&);

      private:
        VulcantVWindow& window;

        std::function<void(const VulcantInputValue&)> onInput = [](const VulcantInputValue&) {};
    };
}