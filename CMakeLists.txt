cmake_minimum_required(VERSION 3.24...4.0.2)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

project(Solution)

include(FetchContent)

#https://thatonegamedev.com/cpp/cmake/godot-4-gdextension-for-c-using-cmake/
FetchContent_Declare(
        GDExtension
        GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
        GIT_TAG godot-4.5-stable # <- to change the compatible godot version just change this tag
)

FetchContent_MakeAvailable(GDExtension)

add_subdirectory(Executable)
add_subdirectory(Vulcant)
add_subdirectory(Examples)
add_subdirectory(VulcantG)
add_subdirectory(ShaderLibrary)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Executable)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CustomTargets")

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Executable)