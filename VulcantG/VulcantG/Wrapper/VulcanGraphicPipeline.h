#pragma once

#include "VulcanImage.h"
#include "VulcanShader.h"
#include "Vulcant/Interface/VulcantRasterizationConfig.h"
#include <glm/glm.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <vector>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;

    class VulcanGraphicPipeline
    {
      public:
        VulcanGraphicPipeline(VulcanDevice&                             deviceInput,
                              const std::vector<VulcanShader*>&         shader,
                              const std::vector<VulcanImage*>&          color,
                              VulcanImage*                              depth        = nullptr,
                              VulcanImage*                              stencil      = nullptr,
                              const VulcantRasterizationConfig&         rasterConfig = VulcantRasterizationConfig());
        virtual ~VulcanGraphicPipeline();

        godot::RID getPipeline() const;
        godot::RID getFramebuffer() const;
        godot::RID getShader() const;
        int64_t    getVertexFormat() const;
        glm::uvec2 getExtent() const;
        size_t     getVertexAttributeCount() const;

      private:
        void createFramebuffer(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);
        void createPipeline(const std::vector<VulcanShader*>&         shader,
                            const std::vector<VulcanImage*>&          color,
                            VulcanImage*                              depth,
                            VulcanImage*                              stencil,
                            const VulcantRasterizationConfig&         rasterConfig);

        VulcanDevice& device;
        godot::RID    pipeline;
        godot::RID    framebuffer;
        godot::RID    shaderRid;
        int64_t       vertexFormat         = 0;
        glm::uvec2    extent{ 0, 0 };
        size_t        vertexAttributeCount = 0;
    };
}