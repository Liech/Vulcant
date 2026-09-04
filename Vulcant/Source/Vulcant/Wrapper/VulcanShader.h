#pragma once

#include "ShaderBindingDefinition.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulcant::Wrapper
{
    class VulcanDevice;
    struct VertexAttribute;

    class VulcanShader
    {
      public:
        VulcanShader(const std::vector<uint32_t>& spirv, VulcanDevice& device);
        virtual ~VulcanShader();

        std::vector<VkDescriptorSetLayout>& getDescriptorSetLayout();
        VkPipelineLayout&                   getPipelineLayout();
        VkPipeline&                         getComputePipeline();

        VkPipelineShaderStageCreateInfo& getCreateInfo();
        std::vector<VertexAttribute>     getVertexLayout();
        VkShaderStageFlagBits            getStage() const;

        const std::vector<std::vector<ShaderBindingDefinition>>& getLayout() const;

      private:
        void createPipelineLayout();
        void createDescriptorSetLayout();
        void createComputePipeline();
        void createCreateInfo();

        VulcanDevice& device;

        std::vector<std::vector<ShaderBindingDefinition>> layouts;

        std::vector<uint32_t>              spirv;
        VkShaderModule                     shaderModule{};
        std::vector<VkDescriptorSetLayout> descriptorSetLayout{};
        VkPipelineLayout                   pipelineLayout{};
        VkPipeline                         pipeline{};
        VkPipelineShaderStageCreateInfo    createInfo{};
        VkShaderStageFlagBits              stage{};
    };
}