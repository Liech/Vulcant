#pragma once

#include "Vulcant/Interface/VulcantSet.h"
#include <memory>
#include <vector>

namespace Vulcant::Wrapper
{
    class VulcanSet;
    class VulcanShader;
    class VulcanPool;
    struct VulcanResource;
}

namespace Vulcant::VulcantV
{
    class VulcantVSet : public VulcantSet
    {
      public:
        VulcantVSet(const std::vector<std::vector<Wrapper::VulcanResource>>& buffer, Wrapper::VulcanShader& pipeline, Wrapper::VulcanPool& thread);
        virtual ~VulcantVSet();

        virtual void updateResource(size_t set_idx, size_t binding_idx, const VulcantResource& newResource) override;

        std::unique_ptr<Wrapper::VulcanSet> set;
    };
}