#pragma once

#include "Library/Vulcant/Interface/VulcantSet.h"
#include <memory>
#include <vector>

namespace Vulcant::VulcantG
{
    namespace Wrapper
    {
        class VulcanSet;
        class VulcanShader;
        class VulcanDevice;
        struct VulcanResource;
    }

    class VulcantGSet : public VulcantSet
    {
      public:
        VulcantGSet(const std::vector<std::vector<Wrapper::VulcanResource>>& buffer, Wrapper::VulcanShader& shader, Wrapper::VulcanDevice& device);
        virtual ~VulcantGSet();

        virtual void updateResource(size_t set_idx, size_t binding_idx, const VulcantResource& newResource) override;

        std::unique_ptr<Wrapper::VulcanSet> set;
    };
}