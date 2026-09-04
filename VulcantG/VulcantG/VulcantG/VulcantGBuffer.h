#pragma once

#include "Library/Vulcant/Interface/VulcantBuffer.h"
#include <memory>

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanDevice;
        class VulcanBuffer;
    }

    class VulcantGBuffer : public VulcantBuffer
    {
      public:
        VulcantGBuffer(size_t numberOfElements, size_t elementSize, Wrapper::VulcanDevice& device, bool ssbo, bool gpu);
        virtual ~VulcantGBuffer();

        virtual std::unique_ptr<VulcantResource> asResource() const override;
        virtual size_t                           getNumberOfElements() const override;
        virtual size_t                           getElementSize() const override;
        virtual void                             uploadToGPU(const void* data, size_t elementCount, size_t elementOffset = 0) override;
        virtual void                             downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset = 0) override;

        std::unique_ptr<Wrapper::VulcanBuffer> buffer;
    };
}