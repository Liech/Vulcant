#include "VulcantVSet.h"

#include "Library/Vulcant/Interface/VulcantResource.h"
#include "Library/Vulcant/Wrapper/VulcanResource.h"
#include "Library/Vulcant/Wrapper/VulcanSet.h"
#include "Library/Vulcant/VulcantV/VulcantVResource.h"

namespace Vulcant::VulcantV
{
    VulcantVSet::VulcantVSet(const std::vector<std::vector<Wrapper::VulcanResource>>& buffer, Wrapper::VulcanShader& pipeline, Wrapper::VulcanPool& thread)
    {
        set = std::make_unique<Wrapper::VulcanSet>(buffer, pipeline, thread);
    }

    VulcantVSet::~VulcantVSet() {}

    void VulcantVSet::updateResource(size_t set_idx, size_t binding_idx, const VulcantResource& newResource)
    {
        auto& resRef = static_cast<const VulcantVResource&>(newResource).res;
        set->updateResource(set_idx, binding_idx, resRef);
    }
}