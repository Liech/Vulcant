#pragma once

#include <memory>

namespace Vulcant
{
    class VulcantResource;

    class VulcantBuffer
    {
      public:
        VulcantBuffer()          = default;
        virtual ~VulcantBuffer() = default;

        virtual std::unique_ptr<VulcantResource> asResource() const          = 0;
        virtual size_t                           getNumberOfElements() const = 0;
        virtual size_t                           getElementSize() const      = 0;
        virtual void uploadToGPU(const void* data, size_t elementCount, size_t elementOffset = 0) = 0;
        virtual void downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset = 0) = 0;
    };
}