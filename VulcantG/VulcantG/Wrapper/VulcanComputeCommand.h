#pragma once

#include "Library/Vulcant/Interface/VulcantResourceLayout.h"
#include <functional>
#include <glm/glm.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <string>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;
    class VulcanSet;
    class VulcanShader;
    class VulcanBuffer;
    class VulcanImage;

    class VulcanComputeCommand
    {
      public:
        VulcanComputeCommand(VulcanDevice& device);
        virtual ~VulcanComputeCommand();

        void startRecord();
        void add(const godot::Vector3i& groupCount, const VulcanSet& set, VulcanShader& shader);
        void addCopyBuffer(VulcanBuffer& source, VulcanBuffer& dest, size_t elementCount, size_t sourceOffset = 0, size_t destOffset = 0);
        void uploadImageToGPU(VulcanImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset = glm::uvec3(0, 0, 0));
        void downloadImageFromGPU(VulcanImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset);
        void endRecord();

        void runAsync();
        void runSync();
        void wait();

        int64_t getComputeList() const;

      private:
        static std::string addCopyBuffer_shader();

        std::vector<std::function<void()>> beforeComputeQueue; // defer image updates to stay in sync with the behavior of the vulcant api
        std::vector<std::function<void()>> computeListQueue;
        std::vector<std::function<void()>> afterComputeQueue;

        VulcanDevice& device;
        int64_t       computeList = -1;
        bool          isRecording = false;
        bool          isSubmitted = false;
    };
}