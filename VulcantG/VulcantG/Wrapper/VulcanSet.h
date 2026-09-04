#pragma once

#include "VulcanResource.h"
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <vector>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;
    class VulcanShader;

    class VulcanSet
    {
      public:
        VulcanSet(const std::vector<std::vector<VulcanResource>>& resources, VulcanShader& shader, VulcanDevice& device);
        virtual ~VulcanSet();

        const std::vector<godot::RID>& getSets() const;

        void updateResource(size_t set_idx, size_t binding_idx, const VulcanResource& newResource);

        void bind(int64_t compute_list) const;

      private:
        void createDescriptorSet(int target_set_idx = -1);

        VulcanDevice&                            device;
        VulcanShader&                            shader;
        std::vector<std::vector<VulcanResource>> resources;
        std::vector<godot::RID>                  uniformSetRids;
    };
}