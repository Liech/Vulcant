#include "VulcanCompositor.h"
#include <stdexcept>

#include "Library/Vulcant/Interface/VulcantImage.h"
#include "Library/Vulcant/Interface/VulcantComputeCommand.h"
#include "Library/Vulcant/Rendering/BlitShader.h"
#include "VulcantG/VulcantG/VulcantGDevice.h"
#include "VulcantG/VulcantG/VulcantGResource.h"
#include "VulcantG/Wrapper/VulcanDevice.h"
#include "VulcantG/Wrapper/VulcanImage.h"
#include <godot_cpp/classes/compositor.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/world_environment.hpp>

namespace Vulcant::VulcantG::Wrapper
{
    void VulcanCompositor::_bind_methods() {}

    VulcanCompositor::VulcanCompositor()
    {
        device  = std::make_unique<Vulcant::VulcantG::VulcantGDevice>(false);
        blitCmd = device->createComputeCommand();
        blit    = std::make_unique<Vulcant::Rendering::BlitShader>();
        blit->setDevice(*device);
        resolution = glm::ivec2(-1, -1);

        set_effect_callback_type(EFFECT_CALLBACK_TYPE_POST_TRANSPARENT);
        set_enabled(true);
    }

    VulcanCompositor::~VulcanCompositor() {}

    godot::Ref<VulcanCompositor> VulcanCompositor::installCompositorEffect(godot::Node* root_node)
    {
        godot::Ref<VulcanCompositor> newEffect;
        newEffect.instantiate();

        godot::WorldEnvironment* env = getWorld(root_node);

        godot::Ref<godot::Environment> environment_resource = env->get_environment();
        if (environment_resource.is_null())
        {
            environment_resource.instantiate();
            env->set_environment(environment_resource);
        }

        godot::Ref<godot::Compositor> compositor = env->get_compositor();
        if (compositor.is_null())
        {
            compositor.instantiate();
            env->set_compositor(compositor);
        }

        auto effects = compositor->get_compositor_effects();
        effects.push_back(newEffect);
        compositor->set_compositor_effects(effects);
        return newEffect;
    }

    void VulcanCompositor::uninstallCompositorEffect(godot::Node* root_node)
    {
        auto env = getWorld(root_node);

        godot::Ref<godot::Compositor> compositor = env->get_compositor();
        if (compositor.is_null())
            return;

        godot::TypedArray<godot::CompositorEffect> effects = compositor->get_compositor_effects();

        for (int i = 0; i < effects.size(); ++i)
        {
            godot::Variant                      v          = effects[i];
            godot::Ref<godot::CompositorEffect> effect_ref = v;
            if (effect_ref.ptr() == this)
            {
                effects.remove_at(i);
                break;
            }
        }

        compositor->set_compositor_effects(effects);
    }

    godot::WorldEnvironment* VulcanCompositor::getWorld(godot::Node* root_node)
    {
        auto find_env = [&](auto self, godot::Node* current) -> godot::WorldEnvironment*
        {
            if (current->is_class("WorldEnvironment"))
            {
                return Object::cast_to<godot::WorldEnvironment>(current);
            }
            for (int i = 0; i < current->get_child_count(); ++i)
            {
                godot::WorldEnvironment* found = self(self, current->get_child(i));
                if (found)
                    return found;
            }
            return nullptr;
        };

        auto env = find_env(find_env, root_node);

        if (!env)
        {
            env = memnew(godot::WorldEnvironment);
            root_node->add_child(env);
            godot::Ref<godot::Environment> base_env;
            base_env.instantiate();
            env->set_environment(base_env);
        }
        return env;
    }

    VulcantDevice& VulcanCompositor::getDevice()
    {
        return *device;
    }

    void VulcanCompositor::start(const std::function<void()>& onRenderInput, const std::function<void(const glm::ivec2& resolution)>& onResizeInput)
    {
        onRender = onRenderInput;
        onResize = onResizeInput;
    }

    void VulcanCompositor::blitImage(Vulcant::VulcantImage& color)
    {
        assert(screenColor);
        blit->setInputTextures(color, screenColor);
        blit->render();
    }

    void VulcanCompositor::_render_callback(int32_t callback_type, godot::RenderData* render_data)
    {
        if (!render_data)
            return;
        if (callback_type != EFFECT_CALLBACK_TYPE_POST_TRANSPARENT)
            return;

        auto  scene_buffers = render_data->get_render_scene_buffers();
        auto* sb_rd         = godot::Object::cast_to<godot::RenderSceneBuffersRD>(scene_buffers.ptr());
        if (!sb_rd)
        {
            return;
        }

        glm::ivec2 currentSize = { sb_rd->get_internal_size().x, sb_rd->get_internal_size().y };
        if (resolution != currentSize)
        {
            resolution = currentSize;
            onResize(resolution);
        }

        godot::RID screen_color_rid = sb_rd->get_color_layer(0);

        screenColor           = std::make_shared<Vulcant::VulcantG::VulcantGResource>();
        screenColor->res.rid  = screen_color_rid;
        screenColor->res.type = godot::RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE;

        onRender();
        screenColor = nullptr;
    }
}