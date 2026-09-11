#pragma once

#include "Vulcant/Interface/VulcantInputValue.h"
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

struct GLFWwindow;

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanPool;
    class VulcanImage;

    class Window
    {
      public:
        Window(VulcanDevice& device, VulcanPool& pool, const glm::ivec2& resolution, const std::string& title, unsigned int numberOfSwapchainImages = 2);
        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;
        ~Window();

        bool isClosed() const;
        void start(
          std::function<void(double delta)>                            onLogic  = [](double delta) {},
          std::function<void()>                                        onRender = []() {},
          std::function<void(const glm::ivec2& resolution)>            onResize = [](const glm::ivec2& resolution) {},
          std::function<void(const Vulcant::VulcantInputValue& input)> onInput  = [](const Vulcant::VulcantInputValue& input) {});
        void       tick();
        void       blitImage(VulcanImage& color); // color: VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        glm::ivec2 getResolution() const;
        bool       isPressed(const VulcantInputValue&) const;
        glm::dvec2 getMousePosition() const;
        void       setMousePosition(const glm::dvec2&) const;
        GLFWwindow* getGlfwWindow() const;

        static std::vector<std::string> getVulkanExtensions();

      private:
        void pollEvents();
        void swapBuffers();
        void initSwapChain();
        void initWindow(const std::string title);
        void initInput();
        void recreateSwapChain();

        class pimpl;
        std::unique_ptr<pimpl>                p;
        inline static int                     GLFWWindowCount    = 0;
        inline static std::string             crashmessage       = "";
        bool                                  framebufferResized = false;
        std::vector<VulcantInputValue>        inputQueue;
        glm::dvec2                            mousePosition;
        std::chrono::steady_clock::time_point lastTime;
    };

}