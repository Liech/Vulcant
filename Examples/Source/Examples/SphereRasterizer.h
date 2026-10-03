#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Examples/Example.h"
#include "Examples/SphereRasterizerScenes/SphereScene.h"
#include "Rendering/SceneData.h"
namespace Vulcant
{
    class VulcantDevice;
    class VulcantImage;
    class VulcantShader;
    class VulcantSet;
    class VulcantBuffer;
    class VulcantWindow;
    class VulcantComputeCommand;
    class VulcantGraphicCommand;
    class VulcantGraphicPipeline;
    class VulcantUi;
    namespace Rendering
    {
        struct Light;
        class DeferredShading;
        class Freecam;
    }
}

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

struct SphereAnimUniforms
{
    float    time;
    float    animSpeed;
    float    baseRadius;
    uint32_t activeSphereCount;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
};

struct VkDrawIndirectCommandCPU
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct CullParams
{
    uint32_t activeSphereCount;
    uint32_t enableCulling;
    uint32_t enableZSorting;
    uint32_t pad0;
};

struct FrameResources
{
    // Commands
    std::unique_ptr<Vulcant::VulcantGraphicCommand> cmdG;
    std::unique_ptr<Vulcant::VulcantGraphicCommand> uiCmdG;
    std::unique_ptr<Vulcant::VulcantComputeCommand> defcmd;
    std::unique_ptr<Vulcant::VulcantComputeCommand> animCmd;
    std::unique_ptr<Vulcant::VulcantComputeCommand> cullcmd;

    // Buffers
    std::unique_ptr<Vulcant::VulcantBuffer> sceneDataUbo;
    std::unique_ptr<Vulcant::VulcantBuffer> spheresBuffer;
    std::unique_ptr<Vulcant::VulcantBuffer> animUbo;
    std::unique_ptr<Vulcant::VulcantBuffer> culledSpheresBuffer;
    std::unique_ptr<Vulcant::VulcantBuffer> indirectDrawBuffer;
    std::unique_ptr<Vulcant::VulcantBuffer> cullParamsUbo;
    std::unique_ptr<Vulcant::VulcantBuffer> sortBuffer;

    // Sets
    std::unique_ptr<Vulcant::VulcantSet> graphicSet;
    std::unique_ptr<Vulcant::VulcantSet> cullSet;
    std::unique_ptr<Vulcant::VulcantSet> animSet;
};

class SphereRasterizer : public Vulcant::Examples::Example
{
  public:
    static void demo();

    SphereRasterizer();
    virtual ~SphereRasterizer();

    virtual std::string getName() override
    {
        return "SphereRasterizer";
    }

    virtual std::string getDescription() override
    {
        return "High-performance instanced dynamic sphere rasterization";
    }

    Vulcant::VulcantImage& getColor();
    Vulcant::VulcantImage& getDepth();
    Vulcant::VulcantImage& getNormal();

    virtual void                    createWindow(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
    virtual void                    prepare(Vulcant::VulcantDevice& deviceInput, const glm::ivec2& resolution) override;
    virtual void                    changeResolution(const glm::ivec2& newResolution) override;
    virtual void                    record(Vulcant::VulcantComputeCommand& cmd) override;
    virtual void                    prepareRun() override;
    virtual Vulcant::VulcantImage&  getResult() override;
    virtual Vulcant::VulcantWindow& getWindow() override;

    void updateSpheres(float time, float dt);
    void regenerateSpheres(size_t count);

    std::vector<Vulcant::Rendering::Light> lightData;

  private:
    Vulcant::Rendering::Light getExampleLight();

    glm::ivec2 resolution = glm::ivec2(-1, -1);

    Vulcant::VulcantDevice* device = nullptr;

    std::vector<SphereData>     spheres;
    std::vector<SphereInitData> sphereInitData;

    std::vector<std::unique_ptr<Vulcant::Examples::SphereScene>> scenes;
    int                                                           currentSceneIndex = 0;

    // Simulation / Rendering parameters
    static constexpr size_t MAX_SPHERES = 50000000;
    int   activeSphereCount = 50000;
    float baseRadius        = 0.05f;
    float animSpeed         = 1.0f;
    bool  animate           = true;
    bool  prevAnimate       = true;
    bool  enableCulling     = true;
    bool  enableZSorting   = true;
    float elapsedTime       = 0.0f;

    void syncAnimationBuffers();

    std::vector<FrameResources> frameResources;
    uint32_t                    currentFrame = 0;

    std::unique_ptr<Vulcant::Rendering::DeferredShading> deferred;
    std::unique_ptr<Vulcant::VulcantBuffer>              spheresBuffer;
    std::unique_ptr<Vulcant::VulcantBuffer>              sphereInitBuffer;

    std::unique_ptr<Vulcant::VulcantShader>              animShader;
    std::unique_ptr<Vulcant::VulcantShader>              cullShader;
    std::unique_ptr<Vulcant::VulcantShader>              cullClearShader;
    std::unique_ptr<Vulcant::VulcantShader>              cullCountShader;
    std::unique_ptr<Vulcant::VulcantShader>              cullPrefixShader;
    std::unique_ptr<Vulcant::VulcantShader>              cullScatterShader;
    std::unique_ptr<Vulcant::VulcantShader>              vertShader;
    std::unique_ptr<Vulcant::VulcantShader>              fragShader;
    std::unique_ptr<Vulcant::VulcantGraphicPipeline>     pipeline;

    std::unique_ptr<Vulcant::VulcantImage>               color;
    std::unique_ptr<Vulcant::VulcantImage>               depth;
    std::unique_ptr<Vulcant::VulcantImage>               normal;

    std::unique_ptr<Vulcant::VulcantWindow>              window;
    std::unique_ptr<Vulcant::VulcantUi>                  ui;
    std::unique_ptr<Vulcant::Rendering::Freecam>         cam;
};
