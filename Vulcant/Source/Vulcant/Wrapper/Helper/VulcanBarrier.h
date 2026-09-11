#pragma once

#include "Vulcant/Interface/VulcantResourceLayout.h"
#include <map>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanCommand;
    class VulcanImage;
    class VulcanBuffer;

    class VulcanBarrier
    {
      public:
        VulcanBarrier(VulcanCommand&);
        virtual ~VulcanBarrier();

        VkTimelineSemaphoreSubmitInfo&  getTimeline();
        const std::vector<VkSemaphore>& getSemaphores();

        void addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& dest);
        void addBarrier(VulcanImage& inputImg, const VulcantResourceLayout& src, const VulcantResourceLayout& dest);
        void addBarrier(VulcanBuffer& buffer);

        static VkImageLayout  mapLayout(VulcantResourceLayout layout);
        VulcantResourceLayout getCurrentLayout(VulcanImage& input);

      private:
        VulcanCommand& cmd;

        bool           created = false;

        VkTimelineSemaphoreSubmitInfo            timelineInfo;
        std::vector<VkSemaphore>                 semaphores;
        std::set<VulcanImage*>                   tracked;
        std::vector<uint64_t>                    waitValues;
        std::vector<uint64_t>                    signalValues;
        std::map<VkImage, VulcantResourceLayout> formerLayouts;
    };
}