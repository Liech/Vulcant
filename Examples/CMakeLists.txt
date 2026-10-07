cmake_minimum_required(VERSION 3.24...4.0.2)

project(Examples)

set(Rendering_Files
    "Source/Rendering/SceneData.h"
    "Source/Rendering/DeferredShading.h"
    "Source/Rendering/DeferredShading.cpp"
    "Source/Rendering/BlitShader.h"
    "Source/Rendering/BlitShader.cpp"
    "Source/Rendering/BlitDepthAwareShader.h"
    "Source/Rendering/BlitDepthAwareShader.cpp"
    "Source/Rendering/SphereData.h"
    "Source/Rendering/SphereRenderer.h"
    "Source/Rendering/SphereRenderer.cpp"
    "Source/Rendering/CubeRenderer.h"
    "Source/Rendering/CubeRenderer.cpp"
    "Source/Rendering/Light.h"
    "Source/Rendering/Freecam.h"
    "Source/Rendering/Freecam.cpp"
)
source_group("Source/Rendering" FILES ${Rendering_Files})


set(SphereScene_Files
    "Source/Examples/SphereRasterizerScenes/SphereScene.h"
    "Source/Examples/SphereRasterizerScenes/FloatingSpheresScene.h"
    "Source/Examples/SphereRasterizerScenes/FloatingSpheresScene.cpp"
    "Source/Examples/SphereRasterizerScenes/CubeGridScene.h"
    "Source/Examples/SphereRasterizerScenes/CubeGridScene.cpp"
    "Source/Examples/SphereRasterizerScenes/ConcentricRingsScene.h"
    "Source/Examples/SphereRasterizerScenes/ConcentricRingsScene.cpp"
    "Source/Examples/SphereRasterizerScenes/HillLandscapeScene.h"
    "Source/Examples/SphereRasterizerScenes/HillLandscapeScene.cpp"
)
source_group("Source/Examples/SphereRasterizerScenes" FILES ${SphereScene_Files})

set(Example_Files
    "Source/Examples/MandelbrotImage.cpp"
    "Source/Examples/MandelbrotImage.h"
    "Source/Examples/SphereImage.cpp"
    "Source/Examples/SphereImage.h"
    "Source/Examples/CubeCaster.cpp"
    "Source/Examples/CubeCaster.h"
    "Source/Examples/CubeCasterFreecam.cpp"
    "Source/Examples/CubeCasterFreecam.h"
    "Source/Examples/Example.cpp"
    "Source/Examples/Example.h"
    "Source/Examples/UiExample.cpp"
    "Source/Examples/UiExample.h"
    "Source/Examples/GraphicPipelineTriangle.cpp"
    "Source/Examples/GraphicPipelineTriangle.h"
    "Source/Examples/GraphicPipelineCube.cpp"
    "Source/Examples/GraphicPipelineCube.h"
    "Source/Examples/SphereRasterizer.cpp"
    "Source/Examples/SphereRasterizer.h"
)
source_group("Source/Examples" FILES ${Example_Files})


set(ALL_FILES
    ${Example_Files}
    ${SphereScene_Files}
    ${Rendering_Files}
)


############################################################################
############################################################################
############################################################################

add_library(Examples ${ALL_FILES})
set_property(TARGET Examples PROPERTY CXX_STANDARD 23)

target_link_libraries(Examples PRIVATE 
Vulcant
ExampleShaderLibrary
)

target_include_directories(Examples PUBLIC 
./Source/
)
