#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanImage;
    class VulcanDevice;
    class VulcanBarrier;
    class VulcanCommand;

    struct StagingResource
    {
        VkBuffer       buffer;
        VkDeviceMemory memory;
    };

    struct DownloadResource
    {
        VkBuffer       buffer;
        VkDeviceMemory memory;
        void*          outCpuPtr; // Target CPU destination address
        size_t         dataSize;  // How many bytes to copy
    };

    class VulcanTransfer
    {
      public:
        VulcanTransfer(VulcanDevice&, VulcanBarrier&, VulcanCommand&);
        virtual ~VulcanTransfer();

        void process();
        void uploadImageToGPU(VulcanImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset = glm::uvec3(0, 0, 0));
        void downloadImageFromGPU(VulcanImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset);


      private:
        VulcanDevice& device;
        VulcanBarrier& barrier;
        VulcanCommand& cmd;

        std::vector<StagingResource>  activeStagingResources;
        std::vector<DownloadResource> activeDownloadResources;
    };
}