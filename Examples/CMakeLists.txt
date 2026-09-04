cmake_minimum_required(VERSION 3.24...4.0.2)

find_package(Catch2 REQUIRED)

project(Examples)

set(Rendering_Files
    "Source/Library/Vulcant/Rendering/SceneData.h"
    "Source/Library/Vulcant/Rendering/DeferredShading.h"
    "Source/Library/Vulcant/Rendering/DeferredShading.cpp"
    "Source/Library/Vulcant/Rendering/BlitShader.h"
    "Source/Library/Vulcant/Rendering/BlitShader.cpp"
    "Source/Library/Vulcant/Rendering/BlitDepthAwareShader.h"
    "Source/Library/Vulcant/Rendering/BlitDepthAwareShader.cpp"
    "Source/Library/Vulcant/Rendering/Light.h"
    "Source/Library/Vulcant/Rendering/Freecam.h"
    "Source/Library/Vulcant/Rendering/Freecam.cpp"
)
source_group("Source/Vulcant/Rendering" FILES ${Rendering_Files})


set(Example_Files
    "Source/Library/Vulcant/Examples/MandelbrotImage.cpp"
    "Source/Library/Vulcant/Examples/MandelbrotImage.h"
    "Source/Library/Vulcant/Examples/SphereImage.cpp"
    "Source/Library/Vulcant/Examples/SphereImage.h"
    "Source/Library/Vulcant/Examples/CubeCaster.cpp"
    "Source/Library/Vulcant/Examples/CubeCaster.h"
    "Source/Library/Vulcant/Examples/CubeCasterFreecam.cpp"
    "Source/Library/Vulcant/Examples/CubeCasterFreecam.h"
    "Source/Library/Vulcant/Examples/Example.cpp"
    "Source/Library/Vulcant/Examples/Example.h"
    "Source/Library/Vulcant/Examples/UiExample.cpp"
    "Source/Library/Vulcant/Examples/UiExample.h"
    "Source/Library/Vulcant/Examples/GraphicPipelineTriangle.cpp"
    "Source/Library/Vulcant/Examples/GraphicPipelineTriangle.h"
    "Source/Library/Vulcant/Examples/GraphicPipelineCube.cpp"
    "Source/Library/Vulcant/Examples/GraphicPipelineCube.h"
)
source_group("Source/Vulcant/Examples" FILES ${Example_Files})


set(ALL_FILES
    ${Example_Files}
    ${Rendering_Files}
)

############################################################################
############################################################################
############################################################################

