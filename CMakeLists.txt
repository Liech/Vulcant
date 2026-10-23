cmake_minimum_required(VERSION 3.24...4.0.2)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

project(Solution)

option(BUILD_GODOT_BINDINGS "Build Godot bindings" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLKIT "Build VulcantToolkit" ON)

if(BUILD_GODOT_BINDINGS)
    include(FetchContent)
    #https://thatonegamedev.com/cpp/cmake/godot-4-gdextension-for-c-using-cmake/
    FetchContent_Declare(
            GDExtension
            GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
            GIT_TAG godot-4.5-stable # <- to change the compatible godot version just change this tag
            GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(GDExtension)
endif()

add_subdirectory(Vulcant)
if(BUILD_TOOLKIT)
    add_subdirectory(VulcantToolkit)
endif()
if(BUILD_EXAMPLES)
    add_subdirectory(ExampleShaderLibrary)
    add_subdirectory(ExampleBrowser)
    add_subdirectory(Examples)
endif()
if(BUILD_GODOT_BINDINGS)
    add_subdirectory(VulcantG)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Executable)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CustomTargets")

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Executable)