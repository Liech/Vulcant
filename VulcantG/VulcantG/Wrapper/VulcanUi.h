#pragma once

#include <chrono>
#include <functional>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Vulcant::VulcantG
{
    class VulcantGWindow;
}

namespace Vulcant::VulcantG::Wrapper
{
    class VulcanDevice;
    class VulcanImage;
    class VulcanGraphicCommand;

    class VulcanUi
    {
      public:
        VulcanUi(VulcanDevice& device, VulcantGWindow* window);
        virtual ~VulcanUi();

        void newFrame();
        void render(VulcanGraphicCommand& cmd, VulcanImage& targetImage);

      private:
        void initPipeline();
        void initFontTexture();
        godot::RID getOrCreateFramebuffer(VulcanImage& targetImage);
        void cleanupFrameResources();

        VulcanDevice&   device;
        VulcantGWindow* window = nullptr;

        godot::RID shaderRid;
        godot::RID pipeline;
        int64_t    vertexFormat = 0;
        int64_t    pipelineFramebufferFormat = -1;

        godot::RID fontTexture;
        godot::RID fontSampler;
        godot::RID fontUniformSet;

        std::unordered_map<uint64_t, godot::RID> framebufferCache;
        std::vector<godot::RID> frameBuffersToFree;

        std::chrono::steady_clock::time_point lastFrameTime;
    };
}
