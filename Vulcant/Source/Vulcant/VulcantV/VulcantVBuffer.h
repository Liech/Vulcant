#pragma once

#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantBufferType.h"
#include <memory>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    class VulcanBuffer;
}

namespace Vulcant::VulcantV
{
    class VulcantVBuffer : public VulcantBuffer
    {
      public:
        VulcantVBuffer(size_t numberOfElements, size_t elementSize, Wrapper::VulcanDevice& device, VulcantBufferType type, bool gpu);
        virtual ~VulcantVBuffer();

        virtual std::unique_ptr<VulcantResource> asResource() const override;
        virtual size_t                           getNumberOfElements() const override;
        virtual size_t                           getElementSize() const override;
        virtual void                             uploadToGPU(const void* data, size_t elementCount, size_t elementOffset = 0) override;
        virtual void                             downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset = 0) override;

        std::unique_ptr<Wrapper::VulcanBuffer> buffer;
    };
}