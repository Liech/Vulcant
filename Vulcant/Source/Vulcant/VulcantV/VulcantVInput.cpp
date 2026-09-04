#include "VulcantVInput.h"

#include "VulcantVWindow.h"

namespace Vulcant::VulcantV
{
    VulcantVInput::VulcantVInput(VulcantVWindow& inputWindow)
      : window(inputWindow)
    {
    }
    VulcantVInput::~VulcantVInput() {}

    void VulcantVInput::setCallback(std::function<void(const VulcantInputValue&)> newInput)
    {
        onInput = newInput;
    }

    void VulcantVInput::call(const VulcantInputValue& v)
    {
        onInput(v);
    }

    glm::dvec2 VulcantVInput::getMousePosition() const
    {
        return window.getMousePosition();
    }

    void VulcantVInput::setMousePosition(const glm::dvec2& pos) const
    {
        window.setMousePosition(pos);
    }

    bool VulcantVInput::isPressed(const VulcantInputValue& key) const
    {
        return window.isPressed(key);
    }
}