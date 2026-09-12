cmake_minimum_required(VERSION 3.24...4.0.2)
project(VulcantG)



set(VulcantG_Wrapper_Files
    "VulcantG/Wrapper/VulcanDevice.h"
    "VulcantG/Wrapper/VulcanDevice.cpp"
    "VulcantG/Wrapper/VulcanShader.h"
    "VulcantG/Wrapper/VulcanShader.cpp"
    "VulcantG/Wrapper/VulcanBuffer.h"
    "VulcantG/Wrapper/VulcanBuffer.cpp"
    "VulcantG/Wrapper/VulcanSet.h"
    "VulcantG/Wrapper/VulcanSet.cpp"
    "VulcantG/Wrapper/VulcanResource.h"
    "VulcantG/Wrapper/VulcanComputeCommand.h"
    "VulcantG/Wrapper/VulcanComputeCommand.cpp"
    "VulcantG/Wrapper/VulcanImage.h"
    "VulcantG/Wrapper/VulcanImage.cpp"
    "VulcantG/Wrapper/VulcanCompositor.h"
    "VulcantG/Wrapper/VulcanCompositor.cpp"
    "VulcantG/Wrapper/VulcanNode.h"
    "VulcantG/Wrapper/VulcanNode.cpp"
    "VulcantG/Wrapper/BufferTransfer.glsl"
    "VulcantG/Wrapper/godot2VulcantInputValue.h"
    "VulcantG/Wrapper/godot2VulcantInputValue.cpp"    
    "VulcantG/Wrapper/VulcanGraphicCommand.cpp"    
    "VulcantG/Wrapper/VulcanGraphicCommand.h"    
    "VulcantG/Wrapper/VulcanGraphicPipeline.h"    
    "VulcantG/Wrapper/VulcanGraphicPipeline.cpp"    
    "VulcantG/Wrapper/VulcanUi.h"
    "VulcantG/Wrapper/VulcanUi.cpp"
)
source_group("Source/VulcantG/Wrapper" FILES ${VulcantG_Wrapper_Files})

set(VulcantG_Files
    "VulcantG/VulcantG/VulcantGDevice.h"
    "VulcantG/VulcantG/VulcantGDevice.cpp"
    "VulcantG/VulcantG/VulcantGBuffer.h"
    "VulcantG/VulcantG/VulcantGBuffer.cpp"
    "VulcantG/VulcantG/VulcantGImage.h"
    "VulcantG/VulcantG/VulcantGImage.cpp"
    "VulcantG/VulcantG/VulcantGShader.h"
    "VulcantG/VulcantG/VulcantGShader.cpp"
    "VulcantG/VulcantG/VulcantGComputeCommand.h"
    "VulcantG/VulcantG/VulcantGComputeCommand.cpp"
    "VulcantG/VulcantG/VulcantGGraphicCommand.h"
    "VulcantG/VulcantG/VulcantGGraphicCommand.cpp"
    "VulcantG/VulcantG/VulcantGSet.h"
    "VulcantG/VulcantG/VulcantGSet.cpp"
    "VulcantG/VulcantG/VulcantGResource.h"
    "VulcantG/VulcantG/VulcantGDevice.h"
    "VulcantG/VulcantG/VulcantGDevice.cpp"
    "VulcantG/VulcantG/VulcantGWindow.h"
    "VulcantG/VulcantG/VulcantGWindow.cpp"
    "VulcantG/VulcantG/VulcantGInput.h"
    "VulcantG/VulcantG/VulcantGInput.cpp"
    "VulcantG/VulcantG/VulcantGGraphicPipeline.h"
    "VulcantG/VulcantG/VulcantGGraphicPipeline.cpp"
    "VulcantG/VulcantG/VulcantGUi.h"
    "VulcantG/VulcantG/VulcantGUi.cpp"
)
source_group("Source/VulcantG/VulcantG" FILES ${VulcantG_Files})


set(Source_Files
    "register_types.h"
    "register_types.cpp"
    "ExampleRunner.h"
    "ExampleRunner.cpp"    
)
source_group("Source" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
    ${VulcantG_Wrapper_Files}
    ${VulcantG_Files}
)

add_library(VulcantG SHARED ${ALL_FILES})
set_property(TARGET VulcantG PROPERTY DBG_POSTFIX d)
set_property(TARGET VulcantG PROPERTY CXX_STANDARD 23)


target_include_directories(VulcantG PRIVATE 
"${CMAKE_CURRENT_SOURCE_DIR}/src"
./../Vulcant/Source
./
)

target_link_libraries(VulcantG PUBLIC 
godot::cpp 
Vulcant
Examples
)

add_custom_command(TARGET VulcantG POST_BUILD 
               COMMAND ${CMAKE_COMMAND} -E copy_directory 
               $<TARGET_FILE_DIR:VulcantG>              
                ${CMAKE_SOURCE_DIR}/godotProject/bin)