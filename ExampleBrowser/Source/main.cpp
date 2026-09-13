#include "Examples/Example.h"
#include <iostream>
#include <limits>
#include <cstdlib>
#include "Vulcant/Wrapper/Window.h"
#include "Vulcant/VulcantV/VulcantVDevice.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantWindow.h"

int main()
{
    auto all = Vulcant::Examples::Example::getAll();

    if (all.empty())
    {
        std::cout << "No examples available.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Available examples:\n";
    for (size_t i = 0; i < all.size(); ++i)
    {
        std::cout << i << ": " << all[i]->getName() << " - " << all[i]->getDescription() << "\n";
    }

    std::cout << "Choose example index: ";
    int index = -1;
    while (!(std::cin >> index) || index < 0 || static_cast<size_t>(index) >= all.size())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid selection. Enter a number between 0 and " << (all.size() - 1) << ": ";
    }

    auto chosen = all[static_cast<size_t>(index)];
    std::cout << "Selected: " << chosen->getName() << "\n";

    all.clear();
    
    {
        auto                              windowExtensions = Vulcant::Wrapper::Window::getVulkanExtensions();
        Vulcant::VulcantV::VulcantVDevice device(windowExtensions);
        {
            chosen->createWindow(device, glm::ivec2(800, 600)); // full window render loop
            auto& window = chosen->getWindow();

            while (!window.isClosed())
            {
                window.tick();
            }
            chosen.reset();
        }
    }

    return EXIT_SUCCESS;
}
