#pragma once

#include "VulcantG/VulcantG/VulcantGImage.h"
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_data.hpp>

namespace Vulcant
{
    class VulcantImage;
    class VulcantDevice;
    class VulcantComputeCommand;
    namespace VulcantG
    {
        class VulcantGDevice;
        class VulcantGResource;
    }
    namespace Rendering
    {
        class BlitShader;
    }
}

namespace godot
{
    class WorldEnvironment;
    class Node;
}

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanImage;
    class VulcanDevice;

    class VulcanCompositor : public godot::CompositorEffect
    {
        GDCLASS(VulcanCompositor, godot::CompositorEffect)
      public:
        VulcanCompositor();
        virtual ~VulcanCompositor();

        VulcantDevice& getDevice();
        void           start(const std::function<void()>& onRender = []() {}, const std::function<void(const glm::ivec2& resolution)>& onResize = [](const glm::ivec2& resolution) {});
        void           blitImage(Vulcant::VulcantImage& color);

        virtual void _render_callback(int32_t p_effect_callback_type, godot::RenderData* p_render_data) override;

        static godot::Ref<VulcanCompositor> installCompositorEffect(godot::Node* root_node);
        void                                uninstallCompositorEffect(godot::Node* root_node);

      private:
        static godot::WorldEnvironment* getWorld(godot::Node* root_node);

        std::unique_ptr<VulcantGDevice>                      device  = nullptr;
        std::unique_ptr<Vulcant::Rendering::BlitShader>      blit    = nullptr;
        std::unique_ptr<Vulcant::VulcantComputeCommand>      blitCmd = nullptr;
        std::shared_ptr<Vulcant::VulcantG::VulcantGResource> screenColor;

        glm::ivec2 resolution;

        std::function<void()>                  onRender;
        std::function<void(const glm::ivec2&)> onResize;

      protected:
        static void _bind_methods();
    };
}