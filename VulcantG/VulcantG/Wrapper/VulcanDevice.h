#pragma once

namespace godot
{
    class RenderingDevice;
}

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice
    {
      public:
        VulcanDevice(bool local = false);
        virtual ~VulcanDevice();

        godot::RenderingDevice& getDevice() const;

        bool isGlobal() const;
    private:
        godot::RenderingDevice* getGlobalDevice() const;
        godot::RenderingDevice* getLocalDevice() const;

        godot::RenderingDevice* device;
        bool                    isLocal = false;
    };
}