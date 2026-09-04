#include "Window.h"
#include "Library/Vulcant/Wrapper/VulcanComputeCommand.h"
#include "Library/Vulcant/Wrapper/VulcanDevice.h"
#include "Library/Vulcant/Wrapper/VulcanImage.h"
#include "Library/Vulcant/Wrapper/VulcanInstance.h"
#include "Library/Vulcant/Wrapper/VulcanPool.h"
#include "Library/Vulcant/VulcantV/VulcantVComputeCommand.h"
#include "Library/Vulcant/VulcantV/VulcantVDevice.h"
#include "Library/Vulcant/VulcantV/VulcantVImage.h"
#include "Helper/glwf2VulcantInputValue.h"

#include <GLFW/glfw3.h>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class Window::pimpl
    {
      public:
        pimpl(Vulcant::Wrapper::VulcanDevice& inputDevice, Vulcant::Wrapper::VulcanPool& inputPool)
          : device(inputDevice)
          , pool(inputPool)
          , cmd(pool, device)
        {
        }

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseCallback(GLFWwindow* window, int button, int action, int mods);

        GLFWwindow*                     window;
        unsigned int                    numberOfSwapchainImages;
        Vulcant::Wrapper::VulcanDevice& device;
        Vulcant::Wrapper::VulcanPool&   pool;
        Vulcant::Wrapper::VulcanComputeCommand cmd;
        VkSurfaceKHR                    surface = nullptr;
        VkSwapchainKHR                  swapchain;
        glm::ivec2                      resolution;
        unsigned int                    currentImageIndex      = 0;
        size_t                          semaphoreAndFenceIndex = 0;

        std::vector<VkImage>                                        swapchainImages;
        std::vector<std::unique_ptr<Vulcant::Wrapper::VulcanImage>> depthImages;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence>     inFlightFences;

        std::function<void(double delta)>                      onLogic  = [](double delta) {};
        std::function<void()>                                  onRender = []() {};
        std::function<void(const glm::ivec2& resolution)>      onResize = [](const glm::ivec2& resolution) {};
        std::function<void(const Vulcant::VulcantInputValue&)> onInput  = [](const Vulcant::VulcantInputValue&) {};

        std::map<Vulcant::VulcantInputValue, bool> isPressedMap;

        const int numberFramesInFlight = 3; // 2 leads to problems
    };

    Window::Window(VulcanDevice& device, VulcanPool& pool, const glm::ivec2& resolution, const std::string& title, unsigned int input_numberOfSwapchainImages)
    {
        p                          = std::make_unique<Window::pimpl>(device, pool);
        p->resolution              = resolution;
        p->numberOfSwapchainImages = input_numberOfSwapchainImages;

        initWindow(title);
        initSwapChain();
        initInput();
        lastTime = std::chrono::steady_clock::now();
    }

    Window::~Window()
    {
        vkDeviceWaitIdle(p->device.getDevice());

        for (size_t i = 0; i < p->imageAvailableSemaphores.size(); i++)
        {
            vkDestroySemaphore(p->device.getDevice(), p->imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(p->device.getDevice(), p->renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(p->device.getDevice(), p->inFlightFences[i], nullptr);
        }

        p->swapchainImages.clear();
        p->depthImages.clear();
        vkDestroySwapchainKHR(p->device.getDevice(), p->swapchain, nullptr);
        vkDestroySurfaceKHR(p->device.getInstance().getInstance(), p->surface, nullptr);

        glfwDestroyWindow(p->window);
        GLFWWindowCount--;

        if (GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
    }

    bool Window::isClosed() const
    {
        return glfwGetWindowAttrib(p->window, GLFW_VISIBLE) == 0 || glfwWindowShouldClose(p->window);
    }

    void Window::tick()
    {
        pollEvents();
        if (glfwGetWindowAttrib(p->window, GLFW_ICONIFIED) != 0)
        {
            glfwWaitEvents();
            return;
        }
        if (framebufferResized)
        {
            vkDeviceWaitIdle(p->device.getDevice());
            recreateSwapChain();
            framebufferResized = false;
            p->pool.reset();
            p->onResize(p->resolution);
            return;
        }
        if (isClosed())
        {
            vkDeviceWaitIdle(p->device.getDevice());
            return;
        }
        int width = 0, height = 0;
        glfwGetFramebufferSize(p->window, &width, &height);
        if (width == 0 || height == 0)
            return;

        glfwGetCursorPos(p->window, &mousePosition.x, &mousePosition.y);

        for (const auto& x : inputQueue)
        {
            p->onInput(x);
        }
        inputQueue.clear();

        auto                          currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed     = currentTime - lastTime;
        double                        deltaTime   = elapsed.count();
        lastTime                                  = currentTime;
        double clampedDelta                       = std::min(deltaTime, 0.1);
        p->onLogic(clampedDelta);

        VK_CHECK_RESULT(vkWaitForFences(p->device.getDevice(), 1, &p->inFlightFences[p->semaphoreAndFenceIndex], VK_TRUE, UINT64_MAX));
        VK_CHECK_RESULT(vkResetFences(p->device.getDevice(), 1, &p->inFlightFences[p->semaphoreAndFenceIndex]));
        VK_CHECK_RESULT(vkAcquireNextImageKHR(p->device.getDevice(), p->swapchain, UINT64_MAX, p->imageAvailableSemaphores[p->semaphoreAndFenceIndex], VK_NULL_HANDLE, &p->currentImageIndex));

        p->onRender();

        swapBuffers();

        if (Window::crashmessage != "")
            throw std::runtime_error(Window::crashmessage);
    }

    void Window::start(std::function<void(double delta)>                            onLogic,
                       std::function<void()>                                        onRender,
                       std::function<void(const glm::ivec2& resolution)>            onResizeInput,
                       std::function<void(const Vulcant::VulcantInputValue& input)> onInput)
    {
        p->onLogic  = onLogic;
        p->onResize = onResizeInput;
        p->onRender = onRender;
        p->onInput  = onInput;
    }

    void Window::pollEvents()
    {
        glfwPollEvents();
    }

    void Window::swapBuffers()
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &p->renderFinishedSemaphores[p->semaphoreAndFenceIndex];
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &p->swapchain;
        presentInfo.pImageIndices      = &p->currentImageIndex;

        VK_CHECK_RESULT(vkQueuePresentKHR(p->device.getQueue(), &presentInfo));

        p->semaphoreAndFenceIndex = (p->semaphoreAndFenceIndex + 1) % p->numberFramesInFlight;
    }

    void Window::initSwapChain()
    {
        if (!p->surface)
            VK_CHECK_RESULT(glfwCreateWindowSurface(p->device.getInstance().getInstance(), p->window, nullptr, &p->surface));

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = p->surface;
        createInfo.minImageCount    = p->numberOfSwapchainImages;
        createInfo.imageFormat      = VK_FORMAT_B8G8R8A8_SRGB;
        createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        createInfo.imageExtent      = VkExtent2D{ (uint32_t)p->resolution.x, (uint32_t)p->resolution.y };
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
        createInfo.clipped          = VK_TRUE;
        createInfo.oldSwapchain     = VK_NULL_HANDLE;

        VK_CHECK_RESULT(vkCreateSwapchainKHR(p->device.getDevice(), &createInfo, nullptr, &p->swapchain));

        uint32_t actualImageCount = 0;
        VK_CHECK_RESULT(vkGetSwapchainImagesKHR(p->device.getDevice(), p->swapchain, &actualImageCount, nullptr));

        std::vector<VkImage> rawImages(actualImageCount);
        VK_CHECK_RESULT(vkGetSwapchainImagesKHR(p->device.getDevice(), p->swapchain, &actualImageCount, rawImages.data()));
        p->numberOfSwapchainImages = actualImageCount;

        for (VkImage img : rawImages)
        {
            p->swapchainImages.push_back(img);

            auto depthImage = std::make_unique<Vulcant::Wrapper::VulcanImage>(p->resolution.x, p->resolution.y, 1, VK_FORMAT_D32_SFLOAT, p->pool, p->device);
            p->depthImages.push_back(std::move(depthImage));
        }

        // Synchronisation
        p->imageAvailableSemaphores.resize(p->numberFramesInFlight);
        p->renderFinishedSemaphores.resize(p->numberFramesInFlight);
        p->inFlightFences.resize(p->numberFramesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < p->inFlightFences.size(); i++)
        {
            VK_CHECK_RESULT(vkCreateSemaphore(p->device.getDevice(), &semaphoreInfo, nullptr, &p->imageAvailableSemaphores[i]));
            VK_CHECK_RESULT(vkCreateSemaphore(p->device.getDevice(), &semaphoreInfo, nullptr, &p->renderFinishedSemaphores[i]));
            VK_CHECK_RESULT(vkCreateFence(p->device.getDevice(), &fenceInfo, nullptr, &p->inFlightFences[i]));
        }
    }

    void Window::pimpl::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        // todo: mods GLFW_MOD_SHIFT
        auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        auto v   = glwfKeyboard2Vulcant(key);
        if (action == GLFW_PRESS)
        {
            app->p->isPressedMap[v] = true;
            app->inputQueue.push_back(v);
        }
        else if (action == GLFW_RELEASE)
        {
            app->p->isPressedMap[v] = false;
        }
    }

    void Window::pimpl::mouseCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        auto v   = glwfMouse2Vulcant(button);
        if (action == GLFW_PRESS)
        {
            app->p->isPressedMap[v] = true;
            app->inputQueue.push_back(v);
        }
        else if (action == GLFW_RELEASE)
        {
            app->p->isPressedMap[v] = false;
        }
    }

    void Window::pimpl::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app                = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void Window::initWindow(const std::string title)
    {
        if (GLFWWindowCount == 0)
        {
            if (!glfwInit())
            {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            glfwSetErrorCallback(
              [](int error, const char* description)
              {
                  std::string msg = "GLFW Error (" + std::to_string(error) + "): " + description;
                  std::cout << msg << std::endl;
                  Window::crashmessage = msg;
              });
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        auto* raw_window = glfwCreateWindow(p->resolution.x, p->resolution.y, title.c_str(), nullptr, nullptr);
        if (!raw_window)
        {
            if (GLFWWindowCount == 0)
            {
                glfwTerminate();
            }
            throw std::runtime_error("Failed to create GLFW window");
        }
        p->window = raw_window;

        glfwSetWindowUserPointer(p->window, this);
        glfwSetFramebufferSizeCallback(p->window, Window::pimpl::framebufferResizeCallback);

        GLFWWindowCount++;
    }

    void Window::initInput()
    {

        glfwSetWindowUserPointer(p->window, this);
        glfwSetKeyCallback(p->window, Window::pimpl::keyCallback);
        glfwSetMouseButtonCallback(p->window, Window::pimpl::mouseCallback);
    }

    std::vector<std::string> Window::getVulkanExtensions()
    {
        std::vector<std::string> result;

        if (!glfwInit())
        {
            throw std::runtime_error("Failed to init GLFW");
        }

        uint32_t                 count = 0;
        const char**             ext   = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> extensions(ext, ext + count);
        for (const auto& x : extensions)
            result.push_back(x);
        return result;
    }

    glm::ivec2 Window::getResolution() const
    {
        return glm::ivec2(p->resolution.x, p->resolution.y);
    }

    void Window::blitImage(VulcanImage& color)
    {
        VkImage  srcImage = color.__getImage();
        VkImage  dstImage = p->swapchainImages[p->currentImageIndex];
        VkDevice device   = p->device.getDevice();
        VkQueue  queue    = p->device.getQueue();

        VkSemaphore imageTimelineSem = color.__getSemaphore();

        uint64_t currentGpuTimelineValue;
        vkGetSemaphoreCounterValue(device, imageTimelineSem, &currentGpuTimelineValue);
        if (currentGpuTimelineValue < color.__getTimeline())
        {
            VkSemaphoreWaitInfo waitInfo{};
            waitInfo.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            waitInfo.semaphoreCount = 1;
            waitInfo.pSemaphores    = &imageTimelineSem;
            waitInfo.pValues        = &color.__getTimeline();
            vkWaitSemaphores(device, &waitInfo, UINT64_MAX);
        }

        p->cmd.startRecord();
        VkCommandBuffer cmdBuffer = p->cmd.__getCommandBuffer();

        VkImageMemoryBarrier barrierDstStart{};
        barrierDstStart.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierDstStart.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
        barrierDstStart.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierDstStart.srcAccessMask    = 0;
        barrierDstStart.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierDstStart.image            = dstImage;
        barrierDstStart.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        VkImageMemoryBarrier barriersStart[1] = { barrierDstStart };
        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, barriersStart);

        VkImageBlit blitRegion{};
        blitRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        blitRegion.srcOffsets[0]  = { 0, 0, 0 };
        blitRegion.srcOffsets[1]  = { static_cast<int32_t>(color.getWidth()), static_cast<int32_t>(color.getHeight()), 1 };

        blitRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        blitRegion.dstOffsets[0]  = { 0, 0, 0 };
        blitRegion.dstOffsets[1]  = { static_cast<int32_t>(p->resolution.x), static_cast<int32_t>(p->resolution.y), 1 };

        vkCmdBlitImage(cmdBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);

        VkImageMemoryBarrier barrierDstEnd{};
        barrierDstEnd.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierDstEnd.oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierDstEnd.newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrierDstEnd.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierDstEnd.dstAccessMask    = 0;
        barrierDstEnd.image            = dstImage;
        barrierDstEnd.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierDstEnd);

        p->cmd.endRecord();

        uint64_t submitWaitValues[]   = { color.__getTimeline(), 0 };
        uint64_t submitSignalValues[] = { color.__getTimeline() + 1, 0 };

        VkTimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.sType                     = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.waitSemaphoreValueCount   = 2;
        timelineInfo.pWaitSemaphoreValues      = submitWaitValues;
        timelineInfo.signalSemaphoreValueCount = 2;
        timelineInfo.pSignalSemaphoreValues    = submitSignalValues;

        VkSemaphore          waitSemaphores[2]   = { imageTimelineSem, p->imageAvailableSemaphores[p->semaphoreAndFenceIndex] };
        VkSemaphore          signalSemaphores[2] = { imageTimelineSem, p->renderFinishedSemaphores[p->semaphoreAndFenceIndex] };
        VkPipelineStageFlags stages[2]           = { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = &timelineInfo;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &cmdBuffer;
        submitInfo.waitSemaphoreCount   = 2;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = stages;
        submitInfo.signalSemaphoreCount = 2;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, p->inFlightFences[p->semaphoreAndFenceIndex]));

        color.__getTimeline()++;
    }

    void Window::recreateSwapChain()
    {
        glfwGetFramebufferSize(p->window, &p->resolution.x, &p->resolution.y);
        while (p->resolution.x == 0 || p->resolution.y == 0)
        {
            glfwGetFramebufferSize(p->window, &p->resolution.x, &p->resolution.y);
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(p->device.getDevice());
        for (size_t i = 0; i < p->imageAvailableSemaphores.size(); i++)
        {
            vkDestroySemaphore(p->device.getDevice(), p->imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(p->device.getDevice(), p->renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(p->device.getDevice(), p->inFlightFences[i], nullptr);
        }
        p->swapchainImages.clear();

        vkDestroySwapchainKHR(p->device.getDevice(), p->swapchain, nullptr);
        initSwapChain();
    }

    bool Window::isPressed(const VulcantInputValue& key) const
    {
        if (p->isPressedMap.contains(key))
            return p->isPressedMap.at(key);
        else
            return false;
    }

    glm::dvec2 Window::getMousePosition() const
    {
        return mousePosition;
    }

    void Window::setMousePosition(const glm::dvec2& pos) const
    {
        glfwSetCursorPos(p->window, pos.x, pos.y);
    }

    GLFWwindow* Window::getGlfwWindow() const
    {
        return p->window;
    }
}
