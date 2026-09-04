#include "VulcantGInput.h"

#include "VulcantGWindow.h"

namespace Vulcant::VulcantG
{
    VulcantGInput::VulcantGInput(VulcantGWindow& windowInput)
      : window(windowInput)
    {
    }

    VulcantGInput::~VulcantGInput() {}

    void VulcantGInput::setCallback(std::function<void(const VulcantInputValue&)> newInput)
    {
        onInput = newInput;
    }

    glm::dvec2 VulcantGInput::getMousePosition() const
    {
        return window.getMousePosition();
    }

    void VulcantGInput::setMouseCallback(std::function<void(const glm::dvec2&)> setMousePosInput)
    {
        setMousePos = setMousePosInput;
    }

    void VulcantGInput::setMousePosition(const glm::dvec2& pos) const
    {
        setMousePos(pos);
    }

    bool VulcantGInput::isPressed(const VulcantInputValue& key) const
    {
        if (isPressedMap.contains(key))
            return isPressedMap.at(key);
        return false;
    }

    void VulcantGInput::set(const VulcantInputValue& key, bool v)
    {
        if (v)
        {
            inputQueue.push_back(key);
        }
        isPressedMap[key] = v;
    }

    void VulcantGInput::tick()
    {
        for (const auto& x : inputQueue)
            onInput(x);
        inputQueue.clear();
    }
}