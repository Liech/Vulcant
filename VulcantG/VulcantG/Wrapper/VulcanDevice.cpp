#include "VulcanDevice.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <iostream>

namespace Vulcant::VulcantG::Wrapper
{

    VulcanDevice::VulcanDevice(bool local)
    {
        isLocal = local;
        if (!local)
            device = getGlobalDevice();
        else
            device = getLocalDevice();
    }

    godot::RenderingDevice& VulcanDevice::getDevice() const
    {
        return *device;
    }

    VulcanDevice::~VulcanDevice()
    {
        if (isLocal)
            memdelete(device);
    }

    bool VulcanDevice::isGlobal() const
    {
        return !isLocal;
    }

    godot::RenderingDevice* VulcanDevice::getGlobalDevice() const
    {
        auto result = godot::RenderingServer::get_singleton()->get_rendering_device();
        if (!result)
        {
            std::cout << "Rendering device not available!" << std::endl;
            std::cout << "When using the OpenGL rendering driver or when running in headless mode, this function always returns null" << std::endl;
            throw std::runtime_error("Not Vulcan!");
        }
        return result;
    }

    godot::RenderingDevice* VulcanDevice::getLocalDevice() const
    {
        auto result = godot::RenderingServer::get_singleton()->create_local_rendering_device();

        if (!result)
        {
            std::cout << "Rendering device not available!" << std::endl;
            std::cout << "When using the OpenGL rendering driver or when running in headless mode, this function always returns null" << std::endl;
            throw std::runtime_error("Not Vulcan!");
        }
        return result;
    }

}