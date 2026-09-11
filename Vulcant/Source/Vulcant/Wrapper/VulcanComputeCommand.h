#pragma once

#include "Vulcant/Interface/VulcantResourceLayout.h"
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanPool;
    class VulcanBuffer;
    class VulcanShader;
    class VulcanSet;
    class VulcanImage;
    class VulcanCommand;
    class VulcanBarrier;
    class VulcanTransfer;


    class VulcanComputeCommand
    {
      public:
        VulcanComputeCommand(VulcanPool& pool, VulcanDevice& device);
        virtual ~VulcanComputeCommand();

        void runSync();
        void runAsync();
        void wait();

        void startRecord();
        void endRecord();
        void add(const glm::ivec3& groupCount, VulcanSet& setInput, VulcanShader& pipelineInput);
        void addCopyBuffer(VulcanBuffer& source, VulcanBuffer& dest, size_t elementCount, size_t sourceOffset = 0, size_t destOffset = 0);
        void uploadImageToGPU(VulcanImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset = glm::uvec3(0, 0, 0));
        void downloadImageFromGPU(VulcanImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset);

        void addBarrier(VulcanBuffer&);

        static void executeSingleTime(VulcanPool& pool, VulcanDevice& device, std::function<void(VkCommandBuffer)> action);
        void        addBarrier(VulcanImage&, const VulcantResourceLayout& dest);

        VkCommandBuffer& __getCommandBuffer();

      private:
        void                  runCommandBuffer();

        VulcantResourceLayout getCurrentLayout(VulcanImage& input) const;

        VulcanDevice&                   device;
        VulcanPool&                     pool;
        std::unique_ptr<VulcanCommand>  cmd;
        std::unique_ptr<VulcanBarrier>  barrier;
        std::unique_ptr<VulcanTransfer> transfer;

        enum class status
        {
            uninitialized,
            started,
            filled,
            ready,
            ongpu
        };

        status currentStatus = status::uninitialized;
    };
}