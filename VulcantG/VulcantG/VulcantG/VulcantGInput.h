#pragma once

#include "Library/Vulcant/Interface/VulcantInput.h"
#include <map>
#include <memory>
#include <vector>

namespace Vulcant::VulcantG
{
    class VulcantGWindow;

    class VulcantGInput : public VulcantInput
    {
      public:
        VulcantGInput(VulcantGWindow& window);
        virtual ~VulcantGInput();

        virtual void       setCallback(std::function<void(const VulcantInputValue&)> newInput);
        virtual glm::dvec2 getMousePosition() const;
        virtual void       setMousePosition(const glm::dvec2&) const;
        virtual bool       isPressed(const VulcantInputValue&) const;

        void tick();

        void set(const VulcantInputValue&, bool);
        void setMouseCallback(std::function<void(const glm::dvec2&)> setMousePos);

      private:
        VulcantGWindow&                               window;

        std::vector<VulcantInputValue>                inputQueue;
        std::map<Vulcant::VulcantInputValue, bool>    isPressedMap;
        std::function<void(const VulcantInputValue&)> onInput = [](const VulcantInputValue&) {};
        std::function<void(const glm::dvec2&)>        setMousePos = [](const glm::dvec2&) {};
    };
}