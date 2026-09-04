cmake_minimum_required(VERSION 3.24...4.0.2)

project(ShaderLibrary)

find_program(SLANG_COMPILER slangc)

set(SHADER_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated_shaders")
file(MAKE_DIRECTORY ${SHADER_GEN_DIR})
set(GENERATED_SHADER_HEADERS "")

set(Source_Files
    "Source/ShaderLibrary/DeferredShader.exe.slang"
    "Source/ShaderLibrary/Blit.exe.slang"
    "Source/ShaderLibrary/Blit_DepthAware.exe.slang"
)
source_group("Source" FILES ${Source_Files})

set(Raycast_Files
    "Source/ShaderLibrary/Raycast/Ray.slang"
    "Source/ShaderLibrary/Raycast/Intersect.slang"
    "Source/ShaderLibrary/Raycast/VolumeAccumulator.slang"
)
source_group("Source/Raycast" FILES ${Raycast_Files})

set(RaycastAccumulator_Files
    "Source/ShaderLibrary/Raycast/Accumulator/MaximumIntensityProjection.slang"
    "Source/ShaderLibrary/Raycast/Accumulator/MinimumIntensityProjection.slang"
    "Source/ShaderLibrary/Raycast/Accumulator/AccumulateIntensity.slang"
    "Source/ShaderLibrary/Raycast/Accumulator/Isosurface.slang"
)
source_group("Source/Raycast/Accumulator" FILES ${RaycastAccumulator_Files})


set(Render_Files
    "Source/ShaderLibrary/Render/Scene.slang"
    "Source/ShaderLibrary/Render/Light.slang"
)
source_group("Source/Render" FILES ${Render_Files})

set(Example_Files
    "Source/ShaderLibrary/Example/Spherecast.exe.slang"
    "Source/ShaderLibrary/Example/Mandelbrot.exe.slang"
    "Source/ShaderLibrary/Example/CubeCaster.exe.slang"
    "Source/ShaderLibrary/Example/SVOCaster.exe.slang"
    "Source/ShaderLibrary/Example/Triangle_vert.exe.slang"
    "Source/ShaderLibrary/Example/Triangle_frag.exe.slang"
    "Source/ShaderLibrary/Example/Cube_vert.exe.slang"
    "Source/ShaderLibrary/Example/Cube_frag.exe.slang"
)
source_group("Source/Example" FILES ${Example_Files})


set(SVO_Files
    "Source/ShaderLibrary/SVO/SVONode.slang"
    "Source/ShaderLibrary/SVO/SVO.slang"
    "Source/ShaderLibrary/SVO/BrickSVONode.slang"
    "Source/ShaderLibrary/SVO/BrickSVO.slang"
    "Source/ShaderLibrary/SVO/Raycast.slang"
)
source_group("Source/SVO" FILES ${SVO_Files})

set(Utility_Files
    "Source/ShaderLibrary/Utility/Convert.slang"
)
source_group("Source/Utility" FILES ${Utility_Files})


set(ALL_SLANG_FILES
    ${Source_Files}
    ${Raycast_Files}
    ${RaycastAccumulator_Files}
    ${Example_Files}
    ${Render_Files}
    ${SVO_Files}
    ${Utility_Files}
)

############################################################################
############################################################################
############################################################################

foreach(SHADER_FILE ${ALL_SLANG_FILES})
    if(SHADER_FILE MATCHES "\\.exe\\.slang$")
        set(ABS_SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
        file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${ABS_SHADER_PATH}")
        
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
                    -I "${CMAKE_CURRENT_SOURCE_DIR}/Source/ShaderLibrary"
            DEPENDS "${ABS_SHADER_PATH}"
            COMMENT "Compiling Slang Entry Point: ${REL_PATH}"
            VERBATIM
        )
        
        list(APPEND GENERATED_SHADER_HEADERS "${OUTPUT_HEADER}")
    else()
    endif()
endforeach()


set(main_Files
    "Source/main.cpp"
)
source_group("" FILES ${main_Files})

set(ALL_FILES
    ${ALL_SLANG_FILES}
	${GENERATED_SHADER_HEADERS}
    ${main_Files}
)

include_directories(${SHADER_GEN_DIR})

add_library(ShaderLibrary ${ALL_FILES})
target_include_directories(ShaderLibrary PUBLIC ${SHADER_GEN_DIR})
