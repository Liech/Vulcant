#include "SphereRenderer.h"

#include "ShaderLibrary/Example/SphereFrustumCull_comp.h"
#include "ShaderLibrary/Example/SphereRasterizer_frag.h"
#include "ShaderLibrary/Example/SphereRasterizer_vert.h"
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
    SphereRenderer::SphereRenderer() {}

    SphereRenderer::SphereRenderer(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res, size_t maxSpheresInput)
    {
        init(deviceInput, res, maxSpheresInput);
    }

    SphereRenderer::~SphereRenderer() {}

    void SphereRenderer::init(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res, size_t maxSpheresInput)
    {
        device     = &deviceInput;
        resolution = res;
        maxSpheres = maxSpheresInput;

        // Buffers
        sceneDataUbo          = device->createUniform(1, sizeof(SceneData));
        cullParamsUbo         = device->createUniform(1, sizeof(CullParams));
        indirectDrawBuffer    = device->createIndirectBuffer(1, sizeof(VkDrawIndirectCommandCPU));
        internalSpheresBuffer = device->createBuffer(maxSpheres, sizeof(SphereData));
        culledSpheresBuffer   = device->createBuffer(maxSpheres, sizeof(SphereData));
        currentSpheresBuffer  = internalSpheresBuffer.get();

        // Shaders
        const auto* cull_slang = SphereFrustumCull_comp_spirv;
        auto        cull_size  = SphereFrustumCull_comp_spirv_sizeInBytes;
        auto        cull_spirv = std::vector<uint32_t>(cull_slang, cull_slang + cull_size / sizeof(uint32_t));
        cullShader             = device->createShader(cull_spirv);

        const auto* vert_slang = SphereRasterizer_vert_spirv;
        auto        vert_size  = SphereRasterizer_vert_spirv_sizeInBytes;
        auto        vert_spirv = std::vector<uint32_t>(vert_slang, vert_slang + vert_size / sizeof(uint32_t));
        vertShader             = device->createShader(vert_spirv);

        const auto* frag_slang = SphereRasterizer_frag_spirv;
        auto        frag_size  = SphereRasterizer_frag_spirv_sizeInBytes;
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

    void SphereRenderer::setResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;

        if (ownsOutputTextures)
        {
            allocateOutputTextures();
        }

        createPipeline();
        updateDescriptorSets();
    }

    void SphereRenderer::allocateOutputTextures()
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

    void SphereRenderer::setOutputTextures(Vulcant::VulcantImage& colorInput, Vulcant::VulcantImage& depthInput, Vulcant::VulcantImage& normalInput)
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

    void SphereRenderer::createPipeline()
    {
        if (!device || !vertShader || !fragShader || !color || !normal || !depth)
            return;

        std::vector<Vulcant::VulcantShader*> shaders          = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorAttachments = { color, normal };

        pipeline = device->createVulcanGraphicPipeline(shaders, colorAttachments, depth, nullptr);
    }

    void SphereRenderer::updateDescriptorSets()
    {
        if (!device || !vertShader || !cullShader || !sceneDataUbo || !currentSpheresBuffer ||
            !culledSpheresBuffer || !indirectDrawBuffer || !cullParamsUbo)
        {
            return;
        }

        // Graphic set for culled / indirect drawing:
        // Set 0: SceneData, Set 1: Culled Spheres
        graphicSet = device->createSet(
            { { sceneDataUbo->asResource() }, { culledSpheresBuffer->asResource() } },
            *vertShader);

        // Graphic set for direct drawing:
        // Set 0: SceneData, Set 1: Input Spheres
        graphicDirectSet = device->createSet(
            { { sceneDataUbo->asResource() }, { currentSpheresBuffer->asResource() } },
            *vertShader);

        // Cull compute set:
        // Set 0: { sceneData, inputSpheres, cullParams, culledSpheres, indirectCommand }
        cullSet = device->createSet(
            { { sceneDataUbo->asResource(),
                currentSpheresBuffer->asResource(),
                cullParamsUbo->asResource(),
                culledSpheresBuffer->asResource(),
                indirectDrawBuffer->asResource() } },
            *cullShader);
    }

    void SphereRenderer::setSceneData(const SceneData& data)
    {
        sceneData = data;
        if (sceneDataUbo)
        {
            sceneDataUbo->uploadToGPU(&sceneData, 1);
        }
        updateCullParams();
    }

    void SphereRenderer::setSpheres(const std::vector<SphereData>& spheres)
    {
        setSpheres(spheres.data(), spheres.size());
    }

    void SphereRenderer::setSpheres(const SphereData* spheres, size_t count)
    {
        size_t uploadCount = std::min(count, maxSpheres);
        activeSphereCount  = uploadCount;

        if (internalSpheresBuffer && spheres && uploadCount > 0)
        {
            internalSpheresBuffer->uploadToGPU(spheres, uploadCount);
        }

        if (currentSpheresBuffer != internalSpheresBuffer.get())
        {
            currentSpheresBuffer = internalSpheresBuffer.get();
            updateDescriptorSets();
        }

        updateCullParams();
    }

    void SphereRenderer::setSpheresBuffer(Vulcant::VulcantBuffer& buffer, size_t count)
    {
        bool bufferChanged   = (currentSpheresBuffer != &buffer);
        currentSpheresBuffer = &buffer;
        activeSphereCount    = count;

        if (bufferChanged)
        {
            updateDescriptorSets();
        }

        updateCullParams();
    }

    void SphereRenderer::setActiveSphereCount(size_t count)
    {
        activeSphereCount = std::min(count, maxSpheres);
        updateCullParams();
    }

    void SphereRenderer::setCullingEnabled(bool enable)
    {
        enableCulling = enable;
        updateCullParams();
    }

    bool SphereRenderer::isCullingEnabled() const
    {
        return enableCulling;
    }

    void SphereRenderer::updateCullParams()
    {
        if (cullParamsUbo)
        {
            CullParams cp{ static_cast<uint32_t>(activeSphereCount), enableCulling ? 1u : 0u, 0, 0 };
            cullParamsUbo->uploadToGPU(&cp, 1);
        }

        if (indirectDrawBuffer)
        {
            VkDrawIndirectCommandCPU drawCmd{ 6, 0, 0, 0 };
            indirectDrawBuffer->uploadToGPU(&drawCmd, 1);
        }
    }

    void SphereRenderer::recordCull(Vulcant::VulcantComputeCommand& cmd)
    {
        if (activeSphereCount == 0 || !cullSet || !cullShader)
            return;

        uint32_t groupCountX = static_cast<uint32_t>((activeSphereCount + 255) / 256);
        cmd.add(glm::ivec3(groupCountX, 1, 1), *cullSet, *cullShader);
        cmd.addBarrier(*culledSpheresBuffer);
        cmd.addBarrier(*indirectDrawBuffer);
    }

    void SphereRenderer::record(Vulcant::VulcantGraphicCommand& cmd)
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
            cmd.draw(6, graphicDirectSet.get(), nullptr, static_cast<uint32_t>(activeSphereCount));
        }

        cmd.endRendering();

        cmd.addBarrier(getColor(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd.addBarrier(getDepth(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
        cmd.addBarrier(getNormal(), Vulcant::VulcantResourceLayout::ShaderReadOnly);
    }

    Vulcant::VulcantImage& SphereRenderer::getColor() const
    {
        return *color;
    }

    Vulcant::VulcantImage& SphereRenderer::getDepth() const
    {
        return *depth;
    }

    Vulcant::VulcantImage& SphereRenderer::getNormal() const
    {
        return *normal;
    }

    Vulcant::VulcantBuffer& SphereRenderer::getSpheresBuffer() const
    {
        return *currentSpheresBuffer;
    }

    Vulcant::VulcantBuffer& SphereRenderer::getCulledSpheresBuffer() const
    {
        return *culledSpheresBuffer;
    }

    Vulcant::VulcantBuffer& SphereRenderer::getIndirectDrawBuffer() const
    {
        return *indirectDrawBuffer;
    }

    size_t SphereRenderer::getActiveSphereCount() const
    {
        return activeSphereCount;
    }

    size_t SphereRenderer::getMaxSpheres() const
    {
        return maxSpheres;
    }
}
