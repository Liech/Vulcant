#include "MeshRenderer.h"

#include "ShaderLibrary/Example/MeshRasterizer_frag.h"
#include "ShaderLibrary/Example/MeshRasterizer_vert.h"
#include "Vulcant/Interface/VulcantBuffer.h"
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
    MeshRenderer::MeshRenderer() {}

    MeshRenderer::MeshRenderer(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res)
    {
        init(deviceInput, res);
    }

    MeshRenderer::~MeshRenderer() {}

    void MeshRenderer::init(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& res)
    {
        device     = &deviceInput;
        resolution = res;

        sceneDataUbo  = device->createUniform(1, sizeof(SceneData));
        meshParamsUbo = device->createUniform(1, sizeof(MeshShaderParams));
        updateMeshParamsBuffer();

        const auto* vertSlang = MeshRasterizer_vert_spirv;
        auto        vertSize  = MeshRasterizer_vert_spirv_sizeInBytes;
        auto        vertSpirv = std::vector<uint32_t>(vertSlang, vertSlang + vertSize / sizeof(uint32_t));
        vertShader            = device->createShader(vertSpirv);

        const auto* fragSlang = MeshRasterizer_frag_spirv;
        auto        fragSize  = MeshRasterizer_frag_spirv_sizeInBytes;
        auto        fragSpirv = std::vector<uint32_t>(fragSlang, fragSlang + fragSize / sizeof(uint32_t));
        fragShader            = device->createShader(fragSpirv);

        if (resolution.x > 0 && resolution.y > 0)
        {
            if (ownsOutputTextures)
            {
                allocateOutputTextures();
            }
            createPipeline();
            updateDescriptorSets();
        }
    }

    void MeshRenderer::setResolution(const glm::ivec2& newResolution)
    {
        resolution = newResolution;

        if (ownsOutputTextures)
        {
            allocateOutputTextures();
        }

        createPipeline();
        updateDescriptorSets();
    }

    void MeshRenderer::allocateOutputTextures()
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

    void MeshRenderer::setOutputTextures(Vulcant::VulcantImage& colorInput, Vulcant::VulcantImage& depthInput, Vulcant::VulcantImage& normalInput)
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

    void MeshRenderer::createPipeline()
    {
        if (!device || !vertShader || !fragShader || !color || !normal || !depth)
            return;

        std::vector<Vulcant::VulcantShader*> shaders          = { vertShader.get(), fragShader.get() };
        std::vector<Vulcant::VulcantImage*>  colorAttachments = { color, normal };

        pipeline = device->createVulcanGraphicPipeline(shaders, colorAttachments, depth, nullptr);
    }

    void MeshRenderer::updateDescriptorSets()
    {
        if (!device || !vertShader || !sceneDataUbo || !meshParamsUbo)
            return;

        graphicSet = device->createSet(
          {
            { sceneDataUbo->asResource(), meshParamsUbo->asResource() }
        },
          *vertShader);
    }

    void MeshRenderer::setSceneData(const SceneData& newSceneData)
    {
        sceneData = newSceneData;
        if (sceneDataUbo)
        {
            sceneDataUbo->uploadToGPU(&sceneData, 1);
        }
    }

    void MeshRenderer::setWireframeMode(WireframeMode mode)
    {
        wireframeMode = mode;
        updateMeshParamsBuffer();
    }

    void MeshRenderer::setWireframeColor(const glm::vec3& colorVal)
    {
        wireframeColor = colorVal;
        updateMeshParamsBuffer();
    }

    MeshRenderer::WireframeMode MeshRenderer::getWireframeMode() const
    {
        return wireframeMode;
    }

    const glm::vec3& MeshRenderer::getWireframeColor() const
    {
        return wireframeColor;
    }

    void MeshRenderer::updateMeshParamsBuffer()
    {
        meshParams.wireframeColor = glm::vec4(wireframeColor, 1.5f);
        meshParams.wireframeMode  = static_cast<int>(wireframeMode);
        if (meshParamsUbo)
        {
            meshParamsUbo->uploadToGPU(&meshParams, 1);
        }
    }

    void MeshRenderer::setVertices(const std::vector<MeshVertex>& vertices)
    {
        if (!device || vertices.empty())
        {
            vertexCount = 0;
            vertexBuffer.reset();
            return;
        }

        vertexCount  = static_cast<uint32_t>(vertices.size());
        vertexBuffer = device->createVertexBuffer(vertexCount, sizeof(MeshVertex), false);
        vertexBuffer->uploadToGPU(vertices.data(), vertexCount, 0);
    }

    void MeshRenderer::setMesh(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, const glm::vec3& colorVal)
    {
        if (positions.empty() || indices.empty())
        {
            vertexCount = 0;
            return;
        }

        std::vector<MeshVertex> vertices;
        vertices.reserve(indices.size());

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            if (i0 >= positions.size() || i1 >= positions.size() || i2 >= positions.size())
                continue;

            const glm::vec3& p0 = positions[i0];
            const glm::vec3& p1 = positions[i1];
            const glm::vec3& p2 = positions[i2];

            glm::vec3 normalVal = glm::cross(p1 - p0, p2 - p0);
            float     len       = glm::length(normalVal);
            if (len > 1e-6f)
            {
                normalVal /= len;
            }
            else
            {
                normalVal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            vertices.push_back({ p0, normalVal, colorVal });
            vertices.push_back({ p1, normalVal, colorVal });
            vertices.push_back({ p2, normalVal, colorVal });
        }

        setVertices(vertices);
    }

    void MeshRenderer::setMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<uint32_t>& indices, const glm::vec3& colorVal)
    {
        if (positions.empty() || indices.empty())
        {
            vertexCount = 0;
            return;
        }

        std::vector<MeshVertex> vertices;
        vertices.reserve(indices.size());

        bool hasNormals = (normals.size() == positions.size());

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            if (i0 >= positions.size() || i1 >= positions.size() || i2 >= positions.size())
                continue;

            const glm::vec3& p0 = positions[i0];
            const glm::vec3& p1 = positions[i1];
            const glm::vec3& p2 = positions[i2];

            glm::vec3 n0 = hasNormals ? normals[i0] : glm::cross(p1 - p0, p2 - p0);
            glm::vec3 n1 = hasNormals ? normals[i1] : n0;
            glm::vec3 n2 = hasNormals ? normals[i2] : n0;

            vertices.push_back({ p0, n0, colorVal });
            vertices.push_back({ p1, n1, colorVal });
            vertices.push_back({ p2, n2, colorVal });
        }

        setVertices(vertices);
    }

    void MeshRenderer::record(Vulcant::VulcantGraphicCommand& cmd)
    {
        if (vertexCount == 0 || !pipeline || !vertexBuffer || !color || !depth || !normal)
            return;

        cmd.beginRendering(*pipeline);
        cmd.setViewportAndScissor(glm::uvec2(resolution.x, resolution.y));
        cmd.draw(vertexCount, graphicSet.get(), vertexBuffer.get());
        cmd.endRendering();
    }

    Vulcant::VulcantImage& MeshRenderer::getColor() const
    {
        return *color;
    }

    Vulcant::VulcantImage& MeshRenderer::getDepth() const
    {
        return *depth;
    }

    Vulcant::VulcantImage& MeshRenderer::getNormal() const
    {
        return *normal;
    }

    uint32_t MeshRenderer::getVertexCount() const
    {
        return vertexCount;
    }

    uint32_t MeshRenderer::getTriangleCount() const
    {
        return vertexCount / 3;
    }
}
