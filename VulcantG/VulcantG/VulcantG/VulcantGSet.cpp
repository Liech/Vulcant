#include "VulcantGSet.h"

#include "Vulcant/Interface/VulcantResource.h"
#include "VulcantG/VulcantG/VulcantGResource.h"
#include "VulcantG/Wrapper/VulcanResource.h"
#include "VulcantG/Wrapper/VulcanSet.h"

namespace Vulcant::VulcantG
{
    VulcantGSet::VulcantGSet(const std::vector<std::vector<Wrapper::VulcanResource>>& buffer, Wrapper::VulcanShader& shader, Wrapper::VulcanDevice& device)
    {
        set = std::make_unique<Wrapper::VulcanSet>(buffer, shader, device);
    }

    VulcantGSet::~VulcantGSet() {}

    void VulcantGSet::updateResource(size_t set_idx, size_t binding_idx, const VulcantResource& newResource)
    {
        auto& resRef = static_cast<const VulcantGResource&>(newResource).res;
        set->updateResource(set_idx, binding_idx, resRef);
    }
}