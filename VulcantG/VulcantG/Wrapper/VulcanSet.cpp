#include "VulcanSet.h"
#include "VulcanDevice.h"
#include "VulcanShader.h"
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace Vulcant::VulcantG::Wrapper
{
    VulcanSet::VulcanSet(const std::vector<std::vector<VulcanResource>>& res, VulcanShader& shaderInput, VulcanDevice& dev)
      : resources(res)
      , shader(shaderInput)
      , device(dev)
    {
        createDescriptorSet();
    }

    VulcanSet::~VulcanSet() {}

    void VulcanSet::createDescriptorSet(int target_set_idx)
    {
        using namespace godot;
        RenderingDevice& rd  = device.getDevice();
        auto             lay = shader.getLayout();

        size_t start_set = (target_set_idx == -1) ? 0 : target_set_idx;
        size_t end_set   = (target_set_idx == -1) ? resources.size() : target_set_idx + 1;

        if (target_set_idx == -1)
        {
            uniformSetRids.resize(resources.size());
        }

        for (size_t set_idx = start_set; set_idx < end_set; ++set_idx)
        {
            TypedArray<RDUniform> uniform_array;

            for (uint32_t binding_idx = 0; binding_idx < resources[set_idx].size(); ++binding_idx)
            {
                const auto& res      = resources[set_idx][binding_idx];
                auto        layoutvk = lay[set_idx][binding_idx].layout;

                Ref<RDUniform> u;
                u.instantiate();
                u->set_binding(binding_idx);

                if (res.type == godot::RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE)
                {
                    if (layoutvk == VK_IMAGE_LAYOUT_GENERAL)
                        u->set_uniform_type(godot::RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE);
                    else
                        u->set_uniform_type(godot::RenderingDevice::UniformType::UNIFORM_TYPE_TEXTURE);
                    u->add_id(res.rid);
                }
                else if (res.type == godot::RenderingDevice::UniformType::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE)
                {
                    u->set_uniform_type(godot::RenderingDevice::UniformType::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
                    u->add_id(res.sampler);
                    u->add_id(res.rid);
                }
                else
                {
                    u->set_uniform_type(res.type);
                    u->add_id(res.rid);
                }
                uniform_array.append(u);
            }

            RID set_rid             = rd.uniform_set_create(uniform_array, shader.getShader(), set_idx);
            uniformSetRids[set_idx] = set_rid;
        }
    }

    const std::vector<godot::RID>& VulcanSet::getSets() const
    {
        return uniformSetRids;
    }

    void VulcanSet::bind(int64_t compute_list) const
    {
        godot::RenderingDevice& rd = device.getDevice();

        for (uint32_t set_idx = 0; set_idx < uniformSetRids.size(); ++set_idx)
        {
            if (uniformSetRids[set_idx].is_valid())
            {
                rd.compute_list_bind_uniform_set(compute_list, uniformSetRids[set_idx], set_idx);
            }
        }
    }

    void VulcanSet::bindDrawList(int64_t draw_list) const
    {
        godot::RenderingDevice& rd = device.getDevice();

        for (uint32_t set_idx = 0; set_idx < uniformSetRids.size(); ++set_idx)
        {
            if (uniformSetRids[set_idx].is_valid())
            {
                rd.draw_list_bind_uniform_set(draw_list, uniformSetRids[set_idx], set_idx);
            }
        }
    }

    void VulcanSet::updateResource(size_t set_idx, size_t binding_idx, const VulcanResource& newResource)
    {
        if (set_idx >= resources.size() || binding_idx >= resources[set_idx].size())
        {
            throw std::runtime_error("Godot Descriptor Update: Index out of bounds!");
        }
        resources[set_idx][binding_idx] = newResource;
        createDescriptorSet(set_idx);
    }
}