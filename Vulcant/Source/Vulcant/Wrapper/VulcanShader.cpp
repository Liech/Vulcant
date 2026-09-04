#include "VulcanShader.h"

#include "ShaderCompiler.h"
#include "VulcanDevice.h"
#include "VulcanInstance.h"

namespace Vulcant::Wrapper
{
    VulcanShader::VulcanShader(const std::vector<uint32_t>& spirvInput, VulcanDevice& deviceInput)
      : device(deviceInput)
      , spirv(spirvInput)
    {
        layouts = ShaderCompiler::getLayout(spirv);
        stage   = ShaderCompiler::getShaderStage(spirv);
        createDescriptorSetLayout();
        createPipelineLayout();
        createCreateInfo();
        if (stage == VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT)
            createComputePipeline();
    }

    VulcanShader::~VulcanShader()
    {
        vkDestroyShaderModule(device.getDevice(), shaderModule, NULL);
        for (auto& x : descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device.getDevice(), x, NULL);
        vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, NULL);
        if (pipeline)
            vkDestroyPipeline(device.getDevice(), pipeline, NULL);
    }

    VkShaderStageFlagBits VulcanShader::getStage() const
    {
        return stage;
    }

    std::vector<VkDescriptorSetLayout>& VulcanShader::getDescriptorSetLayout()
    {
        return descriptorSetLayout;
    }

    VkPipelineLayout& VulcanShader::getPipelineLayout()
    {
        return pipelineLayout;
    }

    VkPipeline& VulcanShader::getComputePipeline()
    {
        assert(stage == VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
        return pipeline;
    }

    void VulcanShader::createCreateInfo()
    {
        uint32_t        filelength = spirv.size() * sizeof(uint32_t);
        const uint32_t* code       = spirv.data();

        VkShaderModuleCreateInfo moduleCreateInfo = {};
        moduleCreateInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pCode                    = code;
        moduleCreateInfo.codeSize                 = filelength;

        VK_CHECK_RESULT(vkCreateShaderModule(device.getDevice(), &moduleCreateInfo, NULL, &shaderModule));

        createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage  = stage;
        createInfo.module = shaderModule;
        createInfo.pName  = "main";
    }

    void VulcanShader::createComputePipeline()
    {
        VkComputePipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType                       = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage                       = createInfo;
        pipelineCreateInfo.layout                      = pipelineLayout;

        VK_CHECK_RESULT(vkCreateComputePipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &pipeline));
    }

    VkPipelineShaderStageCreateInfo& VulcanShader::getCreateInfo()
    {
        return createInfo;
    }

    void VulcanShader::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount             = descriptorSetLayout.size();
        pipelineLayoutCreateInfo.pSetLayouts                = descriptorSetLayout.data();
        VK_CHECK_RESULT(vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutCreateInfo, NULL, &pipelineLayout));
    }

    void VulcanShader::createDescriptorSetLayout()
    {
        descriptorSetLayout.resize(layouts.size());

        for (size_t i = 0; i < descriptorSetLayout.size(); i++)
        {
            std::vector<VkDescriptorSetLayoutBinding> setBindings;
            const auto&                               currentLayoutBindings = layouts[i];

            // Iterate over the inner vector (bindings for the current set)
            for (size_t j = 0; j < currentLayoutBindings.size(); j++)
            {
                const auto& definition = currentLayoutBindings[j];

                VkDescriptorSetLayoutBinding binding = {};

                binding.binding                    = (uint32_t)j;
                binding.descriptorCount = (uint32_t)definition.count; // Use the count from the struct
                binding.descriptorType  = definition.type;            // Use the type from the struct
                binding.stageFlags      = stage;

                setBindings.push_back(binding);
            }

            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
            descriptorSetLayoutCreateInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutCreateInfo.bindingCount                    = (uint32_t)setBindings.size();
            descriptorSetLayoutCreateInfo.pBindings                       = setBindings.data();

            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.getDevice(), &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout[i]));
        }
    }

    std::vector<VertexAttribute> VulcanShader::getVertexLayout()
    {
        assert(stage != VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
        return ShaderCompiler::getVertexInputLayout(spirv);
    }

    const std::vector<std::vector<ShaderBindingDefinition>>& VulcanShader::getLayout() const
    {
        return layouts;
    }
}
