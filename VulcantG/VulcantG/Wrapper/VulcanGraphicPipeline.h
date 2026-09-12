#pragma once

#include "VulcanImage.h"
#include "VulcanShader.h"
#include <glm/glm.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <vector>

namespace Vulcant::VulcantG::Wrapper
{
    class VulcantDevice;

    class VulcanGraphicPipeline
    {
      public:
        VulcanGraphicPipeline(VulcanDevice& deviceInput, const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);
        ~VulcanGraphicPipeline();

        godot::RID getPipeline() const;
        godot::RID getFramebuffer() const;
        godot::RID getShader() const;
        int64_t    getVertexFormat() const;
        glm::uvec2 getExtent() const;

      private:
        VulcanDevice& device;
        godot::RID    pipeline;
        godot::RID    framebuffer;
        godot::RID    shaderRid;
        int64_t       vertexFormat = 0;
        glm::uvec2    extent{ 0, 0 };

        void createFramebuffer(const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);
        void createPipeline(const std::vector<VulcanShader*>& shader, const std::vector<VulcanImage*>& color, VulcanImage* depth, VulcanImage* stencil);
    };
}