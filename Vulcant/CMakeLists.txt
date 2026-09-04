cmake_minimum_required(VERSION 3.24...4.0.2)

find_package(Catch2 REQUIRED)

project(Vulcant)
project(VulcantTest)

find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui REQUIRED)

get_target_property(IMGUI_DIR imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)

set(Wrapper_Files
    "Source/Vulcant/Wrapper/ShaderCompiler.cpp"
    "Source/Vulcant/Wrapper/ShaderCompiler.h"
    "Source/Vulcant/Wrapper/VulcanInstance.cpp"
    "Source/Vulcant/Wrapper/VulcanInstance.h"
    "Source/Vulcant/Wrapper/VulcanDevice.cpp"
    "Source/Vulcant/Wrapper/VulcanDevice.h"
    "Source/Vulcant/Wrapper/VulcanBuffer.cpp"
    "Source/Vulcant/Wrapper/VulcanBuffer.h"
    "Source/Vulcant/Wrapper/VulcanImage.cpp"
    "Source/Vulcant/Wrapper/VulcanImage.h"
    "Source/Vulcant/Wrapper/VulcanShader.cpp"
    "Source/Vulcant/Wrapper/VulcanShader.h"
    "Source/Vulcant/Wrapper/VulcanPool.cpp"
    "Source/Vulcant/Wrapper/VulcanPool.h"
    "Source/Vulcant/Wrapper/VulcanSet.cpp"
    "Source/Vulcant/Wrapper/VulcanSet.h"
    "Source/Vulcant/Wrapper/VulcanComputeCommand.cpp"
    "Source/Vulcant/Wrapper/VulcanComputeCommand.h"
    "Source/Vulcant/Wrapper/VulcanResource.h"
    "Source/Vulcant/Wrapper/ShaderBindingDefinition.h"
    "Source/Vulcant/Wrapper/Window.cpp"
    "Source/Vulcant/Wrapper/Window.h"
)
source_group("Source/Vulcant/Wrapper" FILES ${Wrapper_Files})


set(WrapperGraphic_Files
    "Source/Vulcant/Wrapper/Graphic/GraphicPipelineGenerator.cpp"
    "Source/Vulcant/Wrapper/Graphic/GraphicPipelineGenerator.h"
    "Source/Vulcant/Wrapper/Graphic/VulcanGraphicPipeline.cpp"
    "Source/Vulcant/Wrapper/Graphic/VulcanGraphicPipeline.h"
    "Source/Vulcant/Wrapper/Graphic/VulcanGraphicCommand.cpp"
    "Source/Vulcant/Wrapper/Graphic/VulcanGraphicCommand.h"
)
source_group("Source/Vulcant/Wrapper/Graphic" FILES ${WrapperGraphic_Files})

set(WrapperHelper_Files
    "Source/Vulcant/Wrapper/Helper/VulcanCommand.cpp"
    "Source/Vulcant/Wrapper/Helper/VulcanCommand.h"
    "Source/Vulcant/Wrapper/Helper/VulcanBarrier.cpp"
    "Source/Vulcant/Wrapper/Helper/VulcanBarrier.h"
    "Source/Vulcant/Wrapper/Helper/VulcanTransfer.cpp"
    "Source/Vulcant/Wrapper/Helper/VulcanTransfer.h"
    "Source/Vulcant/Wrapper/Helper/glwf2VulcantInputValue.cpp"
    "Source/Vulcant/Wrapper/Helper/glwf2VulcantInputValue.h"
    "Source/Vulcant/Wrapper/Helper/VulcanUi.cpp"
    "Source/Vulcant/Wrapper/Helper/VulcanUi.h"
)
source_group("Source/Vulcant/Wrapper/Helper" FILES ${WrapperHelper_Files})

set(Vulcant_Interface
    "Source/Vulcant/Interface/VulcantDevice.h"
    "Source/Vulcant/Interface/VulcantBuffer.h"
    "Source/Vulcant/Interface/VulcantImage.h"
    "Source/Vulcant/Interface/VulcantShader.h"
    "Source/Vulcant/Interface/VulcantComputeCommand.h"
    "Source/Vulcant/Interface/VulcantGraphicCommand.h"
    "Source/Vulcant/Interface/VulcantGraphicPipeline.h"
    "Source/Vulcant/Interface/VulcantSet.h"
    "Source/Vulcant/Interface/VulcantResource.h"
    "Source/Vulcant/Interface/VulcantImageFormat.h"
    "Source/Vulcant/Interface/VulcantImageUsage.h"
    "Source/Vulcant/Interface/VulcantResourceLayout.h"
    "Source/Vulcant/Interface/VulcantWindow.h"
    "Source/Vulcant/Interface/VulcantInputValue.h"
    "Source/Vulcant/Interface/VulcantInput.h"
    "Source/Vulcant/Interface/VulcantBufferType.h"
    "Source/Vulcant/Interface/VulcantUi.h"
)
source_group("Source/Vulcant/Interface" FILES ${Vulcant_Interface})

set(VulcantV_Files
    "Source/Vulcant/VulcantV/VulcantVDevice.h"
    "Source/Vulcant/VulcantV/VulcantVDevice.cpp"
    "Source/Vulcant/VulcantV/VulcantVBuffer.h"
    "Source/Vulcant/VulcantV/VulcantVBuffer.cpp"
    "Source/Vulcant/VulcantV/VulcantVImage.h"
    "Source/Vulcant/VulcantV/VulcantVImage.cpp"
    "Source/Vulcant/VulcantV/VulcantVShader.h"
    "Source/Vulcant/VulcantV/VulcantVShader.cpp"
    "Source/Vulcant/VulcantV/VulcantVComputeCommand.h"
    "Source/Vulcant/VulcantV/VulcantVComputeCommand.cpp"
    "Source/Vulcant/VulcantV/VulcantVGraphicCommand.h"
    "Source/Vulcant/VulcantV/VulcantVGraphicCommand.cpp"
    "Source/Vulcant/VulcantV/VulcantVSet.h"
    "Source/Vulcant/VulcantV/VulcantVSet.cpp"
    "Source/Vulcant/VulcantV/VulcantVResource.h"
    "Source/Vulcant/VulcantV/VulcantVWindow.h"
    "Source/Vulcant/VulcantV/VulcantVWindow.cpp"
    "Source/Vulcant/VulcantV/VulcantVInput.h"
    "Source/Vulcant/VulcantV/VulcantVInput.cpp"
    "Source/Vulcant/VulcantV/VulcantVGraphicPipeline.h"
    "Source/Vulcant/VulcantV/VulcantVGraphicPipeline.cpp"
    "Source/Vulcant/VulcantV/VulcantVUi.h"
    "Source/Vulcant/VulcantV/VulcantVUi.cpp"
)
source_group("Source/Vulcant/VulcantV" FILES ${VulcantV_Files})

set(Source_Files
    "Source/testMain.cpp"
)
source_group("Source" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
    ${Wrapper_Files}
    ${WrapperGraphic_Files}
    ${WrapperHelper_Files}
    ${Vulcant_Interface}
    ${VulcantV_Files}
)

############################################################################
############################################################################
############################################################################

add_executable(VulcantTest ${ALL_FILES})
add_library(Vulcant ${ALL_FILES})
set_property(TARGET VulcantTest PROPERTY CXX_STANDARD 23)
set_property(TARGET Vulcant PROPERTY CXX_STANDARD 23)


if(MSVC) # strangly necessary because of heavy templating in openvdb and BrickSVOReader
    target_compile_options(VulcantTest PRIVATE /bigobj)
    target_compile_options(Vulcant PRIVATE /bigobj)
endif()

target_link_libraries(VulcantTest PRIVATE 
Catch2::Catch2
glm::glm
nlohmann_json::nlohmann_json
lodepng
Vulkan::Vulkan
unofficial::shaderc::shaderc
unofficial::spirv-reflect
ShaderVulcant
glfw
imgui::imgui
)

target_link_libraries(Vulcant PUBLIC 
glm::glm
nlohmann_json::nlohmann_json
)

target_link_libraries(Vulcant PRIVATE 
lodepng
Vulkan::Vulkan
unofficial::shaderc::shaderc
unofficial::spirv-reflect
ShaderVulcant
glfw
imgui::imgui
)

target_include_directories(Vulcant PUBLIC 
./Source/
)
target_include_directories(VulcantTest PUBLIC 
./Source/
)

set_property( TARGET VulcantTest PROPERTY COMPILE_DEFINITIONS ISTESTPROJECT=1 )


include(CTest)
include(Catch)
catch_discover_tests(VulcantTest)