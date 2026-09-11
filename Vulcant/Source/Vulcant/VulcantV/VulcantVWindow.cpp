#include "VulcantVWindow.h"

#include "Vulcant/Wrapper/VulcanDevice.h"
#include "Vulcant/Wrapper/Window.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "Vulcant/VulcantV/VulcantVImage.h"
#include "Vulcant/VulcantV/VulcantVInput.h"

namespace Vulcant::VulcantV
{
    VulcantVWindow::VulcantVWindow(Vulcant::VulcantDevice& device, const glm::ivec2& resolution, const std::string& title)
    {
        auto& cast = (VulcantVDevice&)device;
        window     = std::make_unique<Vulcant::Wrapper::Window>(cast.__getDevice(), cast.__getPool(), resolution, title);
        input      = std::make_unique<VulcantVInput>(*this);
    }

    VulcantVWindow::~VulcantVWindow() {}

    void VulcantVWindow::blitImage(Vulcant::VulcantImage& img)
    {
        auto& cast = (VulcantVImage&)img;
        window->blitImage(*cast.img);
    }

    void VulcantVWindow::start(const std::function<void(double)>& onLogic, const std::function<void()>& onRender, const std::function<void(const glm::ivec2& resolution)>& onResize)
    {
        window->start(onLogic, onRender, onResize, [this](const Vulcant::VulcantInputValue& v) { input->call(v); });
    }

    bool VulcantVWindow::isClosed() const
    {
        return window->isClosed();
    }

    void VulcantVWindow::tick()
    {
        window->tick();
    }

    glm::ivec2 VulcantVWindow::getResolution() const
    {
        return window->getResolution();
    }

    Vulcant::VulcantInput& VulcantVWindow::getInput()
    {
        return *input;
    }

    bool VulcantVWindow::isPressed(const VulcantInputValue& key) const
    {
        return window->isPressed(key);
    }

    glm::dvec2 VulcantVWindow::getMousePosition() const
    {
        return window->getMousePosition();
    }

    void VulcantVWindow::setMousePosition(const glm::dvec2& pos)
    {
        window->setMousePosition(pos);
    }

    Wrapper::Window& VulcantVWindow::getWrapper()
    {
        return *window;
    }
}