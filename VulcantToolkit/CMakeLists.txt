cmake_minimum_required(VERSION 3.24...4.0.2)

project(VulcantToolkit)

find_program(SLANG_COMPILER slangc REQUIRED)
find_package(glm REQUIRED)

set(SHADER_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated_shaders")
file(MAKE_DIRECTORY ${SHADER_GEN_DIR})
set(GENERATED_SHADER_HEADERS "")

set(Shader_Files
    "Shaders/ShaderLibrary/Blit.exe.slang"
    "Shaders/ShaderLibrary/Blit_DepthAware.exe.slang"
    "Shaders/ShaderLibrary/DeferredShader.exe.slang"
    "Shaders/ShaderLibrary/Raycast/Ray.slang"
    "Shaders/ShaderLibrary/Render/Scene.slang"
    "Shaders/ShaderLibrary/Render/Light.slang"
    "Shaders/ShaderLibrary/Utility/HalfPacking.slang"
    "Shaders/ShaderLibrary/Example/BillboardRasterizer.slang"
    "Shaders/ShaderLibrary/Example/CubeRasterizer_vert.exe.slang"
    "Shaders/ShaderLibrary/Example/CubeRasterizer_frag.exe.slang"
    "Shaders/ShaderLibrary/Example/MeshRasterizer_vert.exe.slang"
    "Shaders/ShaderLibrary/Example/MeshRasterizer_frag.exe.slang"
    "Shaders/ShaderLibrary/Example/SphereRasterizer_vert.exe.slang"
    "Shaders/ShaderLibrary/Example/SphereRasterizer_frag.exe.slang"
    "Shaders/ShaderLibrary/Example/SphereFrustumCull_comp.exe.slang"
)
source_group("Shaders" FILES ${Shader_Files})

foreach(SHADER_FILE ${Shader_Files})
    if(SHADER_FILE MATCHES "\\.exe\\.slang$")
        set(ABS_SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
        file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Shaders" "${ABS_SHADER_PATH}")

        get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)
        string(REPLACE ".exe.slang" "" BASE_NAME "${REL_PATH}")
        get_filename_component(FILE_NAME_WE "${BASE_NAME}" NAME)

        set(SUB_DIR "${SHADER_GEN_DIR}/${REL_DIR}")
        set(OUTPUT_HEADER "${SUB_DIR}/${FILE_NAME_WE}.h")

        file(MAKE_DIRECTORY "${SUB_DIR}")

        set(STAGE_FLAG "")
        if(SHADER_FILE MATCHES "_vert\\.exe\\.slang$")
            set(STAGE_FLAG "-stage" "vertex")
        elseif(SHADER_FILE MATCHES "_frag\\.exe\\.slang$")
            set(STAGE_FLAG "-stage" "fragment")
        elseif(SHADER_FILE MATCHES "_comp\\.exe\\.slang$")
            set(STAGE_FLAG "-stage" "compute")
        endif()

        add_custom_command(
            OUTPUT "${OUTPUT_HEADER}"
            COMMAND "${SLANG_COMPILER}" "${ABS_SHADER_PATH}"
                    -target spirv
                    -entry main
                    ${STAGE_FLAG}
                    -profile spirv_1_3
                    -source-embed-style u32
                    -source-embed-language cpp
                    -source-embed-name "${FILE_NAME_WE}_spirv"
                    -o "${OUTPUT_HEADER}"
                    -I "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/ShaderLibrary"
            DEPENDS "${ABS_SHADER_PATH}"
            COMMENT "Compiling Slang Entry Point: ${REL_PATH}"
            VERBATIM
        )

        list(APPEND GENERATED_SHADER_HEADERS "${OUTPUT_HEADER}")
    endif()
endforeach()

set(Toolkit_Files
    "Source/Vulcant/Toolkit/ArcCam.h"
    "Source/Vulcant/Toolkit/ArcCam.cpp"
    "Source/Vulcant/Toolkit/BlitDepthAwareShader.h"
    "Source/Vulcant/Toolkit/BlitDepthAwareShader.cpp"
    "Source/Vulcant/Toolkit/BlitShader.h"
    "Source/Vulcant/Toolkit/BlitShader.cpp"
    "Source/Vulcant/Toolkit/Camera.h"
    "Source/Vulcant/Toolkit/CubeRenderer.h"
    "Source/Vulcant/Toolkit/CubeRenderer.cpp"
    "Source/Vulcant/Toolkit/DeferredShading.h"
    "Source/Vulcant/Toolkit/DeferredShading.cpp"
    "Source/Vulcant/Toolkit/Freecam.h"
    "Source/Vulcant/Toolkit/Freecam.cpp"
    "Source/Vulcant/Toolkit/Light.h"
    "Source/Vulcant/Toolkit/MeshRenderer.h"
    "Source/Vulcant/Toolkit/MeshRenderer.cpp"
    "Source/Vulcant/Toolkit/MeshShaderParams.h"
    "Source/Vulcant/Toolkit/MeshShaderParams.cpp"
    "Source/Vulcant/Toolkit/SceneData.h"
    "Source/Vulcant/Toolkit/SphereData.h"
    "Source/Vulcant/Toolkit/SphereRenderer.h"
    "Source/Vulcant/Toolkit/SphereRenderer.cpp"
)
source_group("Source" FILES ${Toolkit_Files})

set(ALL_FILES
    ${Toolkit_Files}
    ${Shader_Files}
    ${GENERATED_SHADER_HEADERS}
)

add_library(VulcantToolkit ${ALL_FILES})
add_library(Vulcant::VulcantToolkit ALIAS VulcantToolkit)

set_property(TARGET VulcantToolkit PROPERTY CXX_STANDARD 20)

target_link_libraries(VulcantToolkit PUBLIC
    Vulcant
    glm::glm
)

target_include_directories(VulcantToolkit PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source/>
    $<BUILD_INTERFACE:${SHADER_GEN_DIR}/>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS VulcantToolkit EXPORT VulcantToolkitTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY Source/ DESTINATION include FILES_MATCHING PATTERN "*.h")
install(EXPORT VulcantToolkitTargets FILE VulcantToolkitTargets.cmake NAMESPACE Vulcant:: DESTINATION share/vulcant)
