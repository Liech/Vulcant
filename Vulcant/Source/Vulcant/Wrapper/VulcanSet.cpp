#include "VulcanSet.h"
#include "VulcanDevice.h"
#include "VulcanInstance.h"
#include "VulcanPool.h"
#include "VulcanResource.h"
#include "VulcanShader.h"
#include <iostream>

namespace Vulcant::Wrapper
{
    VulcanSet::VulcanSet(const std::vector<std::vector<VulcanResource>>& resourceInput, VulcanShader& pipelineInput, VulcanPool& poolInput)
      : resources(resourceInput)
      , pipeline(pipelineInput)
      , pool(poolInput)
    {
        createDescriptorSet();
    }

    VulcanSet::~VulcanSet()
    {
        pool.untrack(*this);
        invalidate();
    }

    void VulcanSet::invalidate()
    {
        if (!valid)
            return;
        // Intuitivly this should be called, but descriptors seem to be optimized in a way that forbids this. Leaving it here to avoid confusion
        // See flag VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        // 
        //if (!descriptorSets.empty())
        //{
        //    vkFreeDescriptorSets(pool.getDevice().getDevice(), pool.getDescriptor(), static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
        //}
        descriptorSets.clear();
        valid = false;
    }

    void VulcanSet::createDescriptorSet()
    {
        pool.track(*this);

        size_t numSets = pipeline.getDescriptorSetLayout().size();
        descriptorSets.resize(numSets);

        size_t amountImages = 0;
        for (size_t layoutI = 0; layoutI < resources.size(); layoutI++)
        {
            for (size_t bindingI = 0; bindingI < resources[layoutI].size(); bindingI++)
            {
                VulcanResource& res = resources[layoutI][bindingI];
                if (res.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE || res.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                    amountImages++;
            }
        }
        imageInfos.resize(amountImages);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool              = pool.getDescriptor();
        allocInfo.descriptorSetCount          = (uint32_t)numSets;
        allocInfo.pSetLayouts                 = pipeline.getDescriptorSetLayout().data();

        // failed to allocate? check VulcanPool::reset
        VK_CHECK_RESULT(vkAllocateDescriptorSets(pool.getDevice().getDevice(), &allocInfo, descriptorSets.data()));

        size_t imgNr = 0;
        for (size_t layoutI = 0; layoutI < resources.size(); layoutI++)
        {
            for (size_t bindingI = 0; bindingI < resources[layoutI].size(); bindingI++)
            {
                VulcanResource& res    = resources[layoutI][bindingI];
                const auto&     layout = pipeline.getLayout()[layoutI][bindingI];

                VkWriteDescriptorSet write = {};
                write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet               = descriptorSets[layoutI];
                write.dstBinding           = (uint32_t)bindingI;
                write.dstArrayElement      = 0;
                write.descriptorCount      = 1;
                write.descriptorType       = res.type;

                if (res.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE || res.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                {
                    auto& info = imageInfos[imgNr];
                    info       = res.imageInfo;
                    // info.imageLayout          = layout.layout;
                    write.pImageInfo = &info;
                    imgNr++;
                }
                else
                {
                    write.pBufferInfo = &res.bufferInfo;
                }

                vkUpdateDescriptorSets(pool.getDevice().getDevice(), 1, &write, 0, NULL);
            }
        }
        valid = true;
    }

    std::vector<VkDescriptorSet>& VulcanSet::getSets()
    {
        if (!valid)
            throw std::runtime_error("Try to return Invalid Set");
        return descriptorSets;
    }

    void VulcanSet::updateResource(size_t layoutI, size_t bindingI, const VulcanResource& newResource)
    {
        if (!valid)
            throw std::runtime_error("Try to update Invalid Set");

        if (layoutI >= resources.size() || bindingI >= resources[layoutI].size())
        {
            throw std::runtime_error("Descriptor Update: Index out of bounds!");
        }

        resources[layoutI][bindingI] = newResource;
        VulcanResource& res          = resources[layoutI][bindingI];

        VkWriteDescriptorSet write = {};
        write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet               = descriptorSets[layoutI];
        write.dstBinding           = (uint32_t)bindingI;
        write.dstArrayElement      = 0;
        write.descriptorCount      = 1;
        write.descriptorType       = res.type;

        if (res.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE || res.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            write.pImageInfo = &res.imageInfo;
        }
        else
        {
            write.pBufferInfo = &res.bufferInfo;
        }
        vkUpdateDescriptorSets(pool.getDevice().getDevice(), 1, &write, 0, nullptr);
    }
}