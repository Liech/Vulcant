#include "VulcanComputeCommand.h"
#include "Vulcant/Interface/VulcantResourceLayout.h"
#include "VulcanBuffer.h"
#include "VulcanDevice.h"
#include "VulcanImage.h"
#include "VulcanSet.h"
#include "VulcanShader.h"
#include <cassert>
#include <godot_cpp/classes/rendering_device.hpp>
#include <iostream>

namespace Vulcant::VulcantG::Wrapper
{
    VulcanComputeCommand::VulcanComputeCommand(VulcanDevice& dev)
      : device(dev)
    {
    }

    VulcanComputeCommand::~VulcanComputeCommand()
    {
        if (isRecording)
            endRecord();
    }

    void VulcanComputeCommand::startRecord()
    {
        assert(computeListQueue.size() == 0);
        assert(!isSubmitted);
        assert(!isRecording);

        computeListQueue.push_back(
          [this]()
          {
              device.getDevice().capture_timestamp("VulcanComputeCommand::startRecord");
              computeList = device.getDevice().compute_list_begin();
          });
        isRecording = true;
        isSubmitted = false;
    }

    void VulcanComputeCommand::add(const godot::Vector3i& groups, const VulcanSet& set, VulcanShader& shader)
    {
        computeListQueue.push_back(
          [this, groups, &set, &shader]()
          {
              auto& rd = device.getDevice();
              shader.bind(computeList);
              set.bind(computeList);
              rd.compute_list_dispatch(computeList, groups.x, groups.y, groups.z);
          });
    }

    void VulcanComputeCommand::endRecord()
    {
        assert(computeListQueue.size() > 0);
        computeListQueue.push_back(
          [this]()
          {
              device.getDevice().compute_list_end();
              device.getDevice().capture_timestamp("VulcanComputeCommand::endRecord");
          });
        isRecording = false;
    }

    void VulcanComputeCommand::runAsync()
    {
        assert(!isRecording && computeListQueue.size() > 0);

        for (const auto& task : beforeComputeQueue)
            task();
        for (const auto& task : computeListQueue)
            task();
        for (const auto& task : afterComputeQueue)
            task();

        if (!device.isGlobal())
        {
            device.getDevice().submit();
        }
        isSubmitted = true;
    }

    void VulcanComputeCommand::runSync()
    {
        runAsync();
        wait();
    }

    void VulcanComputeCommand::wait()
    {
        assert(isSubmitted);
        if (!device.isGlobal())
        {
            device.getDevice().sync();
        }
        isSubmitted = false;
    }

    int64_t VulcanComputeCommand::getComputeList() const
    {
        return computeList;
    }

    std::string VulcanComputeCommand::addCopyBuffer_shader()
    {
        return
#include "BufferTransfer.glsl"
          ;
    }

    void VulcanComputeCommand::addCopyBuffer(VulcanBuffer& source, VulcanBuffer& dest, size_t elementCount, size_t sourceOffset, size_t destOffset)
    {
        using namespace godot;
        assert(isRecording);

        computeListQueue.push_back(
          [this, &source, &dest, elementCount, sourceOffset, destOffset]()
          {
              static RID copyShaderRID   = VulcanShader::compile(addCopyBuffer_shader(), device);
              static RID copyPipelineRID = device.getDevice().compute_pipeline_create(copyShaderRID);

              TypedArray<RDUniform> uniform_array;

              Ref<RDUniform> u_src;
              u_src.instantiate();
              u_src->set_uniform_type(RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER);
              u_src->set_binding(0);
              u_src->add_id(source.getRid());
              uniform_array.append(u_src);

              Ref<RDUniform> u_dst;
              u_dst.instantiate();
              u_dst->set_uniform_type(RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER);
              u_dst->set_binding(1);
              u_dst->add_id(dest.getRid());
              uniform_array.append(u_dst);

              RID transferSet = device.getDevice().uniform_set_create(uniform_array, copyShaderRID, 0);

              PackedInt32Array pc_data;
              pc_data.push_back(static_cast<int32_t>(sourceOffset));
              pc_data.push_back(static_cast<int32_t>(destOffset));
              pc_data.push_back(static_cast<int32_t>(elementCount));

              RenderingDevice& rd = device.getDevice();
              rd.compute_list_bind_compute_pipeline(computeList, copyPipelineRID);
              rd.compute_list_bind_uniform_set(computeList, transferSet, 0);
              rd.compute_list_set_push_constant(computeList, pc_data.to_byte_array(), pc_data.size() * sizeof(int32_t));

              uint32_t x_groups = (static_cast<uint32_t>(elementCount) + 63) / 64;
              rd.compute_list_dispatch(computeList, x_groups, 1, 1);

              rd.compute_list_add_barrier(computeList);
          });
    }

    void VulcanComputeCommand::uploadImageToGPU(VulcanImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset)
    {
        using namespace godot;
        assert(destImage.getRid().is_valid());

        size_t bufferSize = extent.x * extent.y * extent.z * 4;

        PackedByteArray buffer;
        buffer.resize(bufferSize);
        std::memcpy(buffer.ptrw(), cpuData, bufferSize);

        // Queue up to execute right before the compute channel opens
        beforeComputeQueue.push_back([this, &destImage, buffer]() { device.getDevice().texture_update(destImage.getRid(), 0, buffer); });
    }

    void VulcanComputeCommand::downloadImageFromGPU(VulcanImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)
    {
        assert(srcImage.getRid().is_valid());

        // Queue up to execute after the frame has safely processed on the GPU
        afterComputeQueue.push_back(
          [this, &srcImage, outData, extent]()
          {
              godot::RenderingDevice& rd        = device.getDevice();
              godot::PackedByteArray  gpuBuffer = rd.texture_get_data(srcImage.getRid(), 0);

              size_t expectedSize = extent.x * extent.y * extent.z * 4;
              if (gpuBuffer.size() >= expectedSize)
              {
                  std::memcpy(outData, gpuBuffer.ptr(), expectedSize);
              }
          });
    }
}
