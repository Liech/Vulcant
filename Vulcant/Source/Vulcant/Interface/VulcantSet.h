#pragma once

namespace Vulcant
{
    class VulcantResource;

    class VulcantSet
    {
      public:
        VulcantSet()          = default;
        virtual ~VulcantSet() = default;

        virtual void updateResource(size_t set_idx, size_t binding_idx, const VulcantResource& newResource) = 0;
    };
}