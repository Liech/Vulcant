#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Examples/Example.h"
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

struct SphereData
{
    alignas(16) glm::vec3 center;
    float        radius;
    unsigned int colorPacked; // RGBA8 packed
};

struct FrameResources
{
    // Commands
    std::unique_ptr<Vulcant::VulcantGraphicCommand> cmdG;
    std::unique_ptr<Vulcant::VulcantGraphicCommand> uiCmdG;
    std::unique_ptr<Vulcant::VulcantComputeCommand> defcmd;

    // Buffers
    std::unique_ptr<Vulcant::VulcantBuffer> sceneDataUbo;
    std::unique_ptr<Vulcant::VulcantBuffer> spheresBuffer;

    // Sets (da sie auf die eigenen Buffers verweisen)
    std::unique_ptr<Vulcant::VulcantSet> graphicSet;
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
        return "High-performance instanced dynamic sphere rasterization & Gaussian splatting";
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

    std::vector<SphereData> spheres;
    std::vector<glm::vec3>  basePositions;
    std::vector<glm::vec3>  velocities;

    // Simulation / Rendering parameters
    static constexpr size_t MAX_SPHERES = 50000000;
    int   activeSphereCount = 50000;
    float baseRadius        = 0.05f;
    float animSpeed         = 1.0f;
    bool  animate           = true;
    bool  gaussianMode      = false;
    float elapsedTime       = 0.0f;

    std::vector<FrameResources> frameResources;
    uint32_t                    currentFrame = 0;

    std::unique_ptr<Vulcant::Rendering::DeferredShading> deferred;
    std::unique_ptr<Vulcant::VulcantBuffer>              spheresBuffer;

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
