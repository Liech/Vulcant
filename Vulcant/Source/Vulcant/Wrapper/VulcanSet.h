#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanPool;
    class VulcanShader;
    struct VulcanResource;
    class VulcanPool;

    class VulcanSet
    {
      public:
        VulcanSet(const std::vector<std::vector<VulcanResource>>& buffer, VulcanShader& pipeline, VulcanPool& thread);
        virtual ~VulcanSet();

        void updateResource(size_t layoutI, size_t bindingI, const VulcanResource& newResource);

        std::vector<VkDescriptorSet>& getSets();
        void                          invalidate();

      private:
        void createDescriptorSet();

        VulcanPool&                              pool;
        VulcanShader&                            pipeline;
        std::vector<std::vector<VulcanResource>> resources;

        bool                               valid = false;
        std::vector<VkDescriptorSet>       descriptorSets;
        std::vector<VkDescriptorImageInfo> imageInfos;
    };
}