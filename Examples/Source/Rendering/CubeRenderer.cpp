#include "CubeRenderer.h"

#include "ShaderLibrary/Example/CubeRasterizer_frag.h"
#include "ShaderLibrary/Example/CubeRasterizer_vert.h"
#include "ShaderLibrary/Example/SphereFrustumCull_comp.h"
#include "Vulcant/Interface/VulcantBuffer.h"
#include "Vulcant/Interface/VulcantComputeCommand.h"
#include "Vulcant/Interface/VulcantDevice.h"
#include "Vulcant/Interface/VulcantGraphicCommand.h"
#include "Vulcant/Interface/VulcantGraphicPipeline.h"
#include "Vulcant/Interface/VulcantImage.h"
#include "Vulcant/Interface/VulcantResource.h"
#include "Vulcant/Interface/VulcantSet.h"
#include "Vulcant/Interface/VulcantShader.h"

#include <algorithm>
#include <cmath>

namespace Vulcant::Rendering
{
    CubeRenderer::CubeRenderer() {}

    CubeRenderer::CubeRenderer(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res, size_t maxCubesInput)
    {
        init(deviceInput, res, maxCubesInput);
    }

    CubeRenderer::~CubeRenderer() {}

    void CubeRenderer::init(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res, size_t maxCubesInput)
    {
        device     = &deviceInput;
        resolution = res;
        maxCubes   = maxCubesInput;

        // Buffers
        sceneDataUbo        = device->createUniform(1, sizeof(SceneData));
        cullParamsUbo       = device->createUniform(1, sizeof(CullParams));
        indirectDrawBuffer  = device->createIndirectBuffer(1, sizeof(VkDrawIndirectCommandCPU));
        internalCubesBuffer = device->createBuffer(maxCubes, sizeof(SphereData));
        culledCubesBuffer   = device->createBuffer(maxCubes, sizeof(SphereData));
        currentCubesBuffer  = internalCubesBuffer.get();

        // Shaders
        const auto* cull_slang = SphereFrustumCull_comp_spirv;
        auto        cull_size  = SphereFrustumCull_comp_spirv_sizeInBytes;
        auto        cull_spirv = std::vector<uint32_t>(cull_slang, cull_slang + cull_size / sizeof(uint32_t));
        cullShader             = device->createShader(cull_spirv);

        const auto* vert_slang = CubeRasterizer_vert_spirv;
        auto        vert_size  = CubeRasterizer_vert_spirv_sizeInBytes;
        auto        vert_spirv = std::vector<uint32_t>(vert_slang, vert_slang + vert_size / sizeof(uint32_t));
        vertShader             = device->createShader(vert_spirv);

        const auto* frag_slang = CubeRasterizer_frag_spirv;
        auto        frag_size  = CubeRasterizer_frag_spirv_sizeInBytes;
        auto        frag_spirv = std::vector<uint32_t>(frag_slang, frag_slang + frag_size / sizeof(uint32_t));
        fragShader             = device->createShader(frag_spirv);

        if (resolution.x > 0 && resolution.y > 0)
        {
            if (ownsOutputTextures)
            {
                allocateOutputTextures();
            }
            createPipeline();
            updateDescriptorSets();
        }

        updateCullParams();
    }

    void CubeRenderer::setResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;

        if (ownsOutputTextures)
        {
            allocateOutputTextures();
        }

        createPipeline();
        updateDescriptorSets();
    }

    void CubeRenderer::allocateOutputTextures()
    {
        if (!device || resolution.x <= 0 || resolution.y <= 0)
            return;

        internalColor  = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R32G32B32A32_SFLOAT);
        internalNormal = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::R16G16B16A16_SFLOAT);
        internalDepth  = device->createImage(resolution.x, resolution.y, 1, Vulcant::VulcantImageFormat::D32_SFLOAT);

        color  = internalColor.get();
        normal = internalNormal.get();
        depth  = internalDepth.get();
    }

    void CubeRenderer::setOutputTextures(Vulcant::VulcantImage& colorInput, Vulcant::VulcantImage& depthInput, Vulcant::VulcantImage& normalInput)
    {
        ownsOutputTextures = false;
        color              = &colorInput;
        depth              = &depthInput;
        normal             = &normalInput;

        internalColor.reset();
        internalDepth.reset();
        internalNormal.reset();

        createPipeline();
    }

    void CubeRenderer::createPipeline()
    {
        if (!device || !vertShader || !fragShader || !color || !normal || !depth)
            return;

        std::vector<Vulcant::VulcantShader*> shaders          = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorAttachments = { color, normal };

        pipeline = device->createVulcanGraphicPipeline(shaders, colorAttachments, depth, nullptr);
    }

    void CubeRenderer::updateDescriptorSets()
    {
        if (!device || !vertShader || !cullShader || !sceneDataUbo || !currentCubesBuffer ||
            !culledCubesBuffer || !indirectDrawBuffer || !cullParamsUbo)
        {
            return;
        }

        // Graphic set for culled / indirect drawing:
        graphicSet = device->createSet(
            { { sceneDataUbo->asResource() }, { culledCubesBuffer->asResource() } },
            *vertShader);

        // Graphic set for direct drawing:
        graphicDirectSet = device->createSet(
            { { sceneDataUbo->asResource() }, { currentCubesBuffer->asResource() } },
            *vertShader);

        // Cull compute set:
        cullSet = device->createSet(
            { { sceneDataUbo->asResource(),
                currentCubesBuffer->asResource(),
                cullParamsUbo->asResource(),
                culledCubesBuffer->asResource(),
                indirectDrawBuffer->asResource() } },
            *cullShader);
    }

    void CubeRenderer::setSceneData(const SceneData& data)
    {
        sceneData = data;
        if (sceneDataUbo)
        {
            sceneDataUbo->uploadToGPU(&sceneData, 1);
        }
        updateCullParams();
    }

    void CubeRenderer::setCubes(const std::vector<SphereData>& cubes)
    {
        setCubes(cubes.data(), cubes.size());
    }

    void CubeRenderer::setCubes(const SphereData* cubes, size_t count)
    {
        size_t uploadCount = std::min(count, maxCubes);
        activeCubeCount    = uploadCount;

        if (internalCubesBuffer && cubes && uploadCount > 0)
        {
            internalCubesBuffer->uploadToGPU(cubes, uploadCount);
        }

        if (currentCubesBuffer != internalCubesBuffer.get())
        {
            currentCubesBuffer = internalCubesBuffer.get();
            updateDescriptorSets();
        }

        updateCullParams();
    }

    void CubeRenderer::setCubesBuffer(Vulcant::VulcantBuffer& buffer, size_t count)
    {
        bool bufferChanged = (currentCubesBuffer != &buffer);
        currentCubesBuffer = &buffer;
        activeCubeCount    = count;

        if (bufferChanged)
        {
            updateDescriptorSets();
        }

        updateCullParams();
    }

    void CubeRenderer::setActiveCubeCount(size_t count)
    {
        activeCubeCount = std::min(count, maxCubes);
        updateCullParams();
    }

    void CubeRenderer::setCullingEnabled(bool enable)
    {
        enableCulling = enable;
        updateCullParams();
    }

    bool CubeRenderer::isCullingEnabled() const
    {
        return enableCulling;
    }

    void CubeRenderer::updateCullParams()
    {
        if (cullParamsUbo)
        {
            CullParams cp{ static_cast<uint32_t>(activeCubeCount), enableCulling ? 1u : 0u, 0, 0 };
            cullParamsUbo->uploadToGPU(&cp, 1);
        }

        if (indirectDrawBuffer)
        {
            VkDrawIndirectCommandCPU drawCmd{ 6, 0, 0, 0 }; // 6 vertices for billboard quad
            indirectDrawBuffer->uploadToGPU(&drawCmd, 1);
        }
    }

    void CubeRenderer::recordCull(Vulcant::VulcantComputeCommand& cmd)
    {
        if (activeCubeCount == 0 || !cullSet || !cullShader)
            return;

        uint32_t groupCountX = static_cast<uint32_t>((activeCubeCount + 255) / 256);
        cmd.add(glm::ivec3(groupCountX, 1, 1), *cullSet, *cullShader);
        cmd.addBarrier(*culledCubesBuffer);
        cmd.addBarrier(*indirectDrawBuffer);
    }

    void CubeRenderer::record(Vulcant::VulcantGraphicCommand& cmd)
    {
        if (!pipeline)
            return;

        cmd.beginRendering(*pipeline);
        cmd.setViewportAndScissor(glm::uvec2(resolution.x, resolution.y));

        if (enableCulling)
        {
            cmd.drawIndirect(*indirectDrawBuffer, graphicSet.get(), nullptr);
        }
        else
        {
            cmd.draw(6, graphicDirectSet.get(), nullptr, static_cast<uint32_t>(activeCubeCount));
        }

        cmd.endRendering();

        cmd.addBarrier(getColor(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd.addBarrier(getDepth(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd.addBarrier(getNormal(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
    }

    Vulcant::VulcantImage& CubeRenderer::getColor() const
    {
        return *color;
    }

    Vulcant::VulcantImage& CubeRenderer::getDepth() const
    {
        return *depth;
    }

    Vulcant::VulcantImage& CubeRenderer::getNormal() const
    {
        return *normal;
    }

    Vulcant::VulcantBuffer& CubeRenderer::getCubesBuffer() const
    {
        return *currentCubesBuffer;
    }

    Vulcant::VulcantBuffer& CubeRenderer::getCulledCubesBuffer() const
    {
        return *culledCubesBuffer;
    }

    Vulcant::VulcantBuffer& CubeRenderer::getIndirectDrawBuffer() const
    {
        return *indirectDrawBuffer;
    }

    size_t CubeRenderer::getActiveCubeCount() const
    {
        return activeCubeCount;
    }

    size_t CubeRenderer::getMaxCubes() const
    {
        return maxCubes;
    }
}
