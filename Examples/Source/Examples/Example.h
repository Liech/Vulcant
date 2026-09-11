#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Vulcant
{
    class VulcantImage;
    class VulcantDevice;
    class VulcantComputeCommand;
    class VulcantWindow;
}

namespace Vulcant::Examples
{
    class Example
    {
      public:
        static std::vector<std::shared_ptr<Example>> getAll();

        virtual std::string             getName()                                                                       = 0;
        virtual std::string             getDescription()                                                                = 0;
        virtual void createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) = 0;
        virtual void                    prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution)      = 0;
        virtual void                    changeResolution(const glm::ivec2& newResolution)                               = 0;
        virtual void                    record(Vulcant::VulcantComputeCommand& cmd)                                            = 0;
        virtual void                    prepareRun()                                                                    = 0;
        virtual Vulcant::VulcantImage&  getResult()                                                                     = 0;
        virtual Vulcant::VulcantWindow& getWindow()                                                                     = 0;
    };
}