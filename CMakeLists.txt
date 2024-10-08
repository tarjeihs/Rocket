cmake_minimum_required(VERSION 3.16)
project(Rocket)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use only Debug, Release, and Dist configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

# This makes sure that CMake generates the appropriate files for VSCode
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CMakeToolsHelpers OPTIONAL)

# Output directories
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/Binaries)
set(INTERMEDIATE_DIR ${CMAKE_BINARY_DIR}/Intermediate)

# Specify the paths to the GLM and spdlog headers
set(GLM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/glm")
set(SPDLOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/spdlog")
set(VMA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/VulkanMemoryAllocator")
set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/glfw")
set(IMGUI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/imgui")
set(ENTT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/entt")
set(TINYGLTF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/tinygltf")
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/rapidjson")

# Collect ImGui source files
set(IMGUI_SRC
    ${IMGUI_INCLUDE_DIR}/imgui.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
    ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Collect all source files for the Engine project
file(GLOB_RECURSE ENGINE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.h
)
# Collect all HLSL and GLSL files for the Engine project
file(GLOB_RECURSE ENGINE_SHADER_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Shaders/*.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Shaders/*.glsl
)

# Define source groups for Engine
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source PREFIX "Source" FILES ${ENGINE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Shaders PREFIX "Shaders" FILES ${ENGINE_SHADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/imgui PREFIX "Third Party\\ImGui" FILES ${IMGUI_SRC})

# Vulkan SDK
find_package(Vulkan REQUIRED)

# Add GLFW as a subdirectory to be compiled alongside this project
add_subdirectory(Vendor/glfw)

# Target Engine project
add_library(Engine STATIC 
    ${ENGINE_FILES}
    ${IMGUI_SRC}  # Add ImGui source files to the Engine target
)

set_source_files_properties(${ENGINE_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(Engine PRIVATE ${ENGINE_SHADER_FILES})

target_include_directories(Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source
    ${GLM_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR}/include
    ${VMA_INCLUDE_DIR}/include
    ${GLFW_INCLUDE_DIR}/include
    ${IMGUI_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}/backends
    ${ENTT_INCLUDE_DIR}/src
    ${TINYGLTF_INCLUDE_DIR}
    ${RAPIDJSON_INCLUDE_DIR}/include
)

target_precompile_headers(Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/EnginePCH.h)

if(UNIX AND NOT APPLE)
    target_link_libraries(Engine PRIVATE
        glfw
        Vulkan::Vulkan
        X11
        X11-xcb
    )
else()
    target_link_libraries(Engine PRIVATE
        glfw
        Vulkan::Vulkan
    )
endif()

target_compile_definitions(Engine PRIVATE
    _CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_VULKAN
)

# Platform-specific settings
if(MSVC)
    add_compile_options(/MP)
    target_compile_options(Engine PRIVATE /std:c++20)

    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DIST "/MD /O2 /Ob2 /DNDEBUG")

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
    set(CMAKE_EXE_LINKER_FLAGS_DIST "")

    target_compile_definitions(Engine PRIVATE
        RK_PLATFORM_WINDOWS
        RK_BUILD_DLL
        RK_ENGINE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/Engine\"
    )
else()
    # Non-MSVC compilers (GCC, Clang)
    target_compile_options(Engine PRIVATE -std=c++20)

    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DIST "-O2 -DNDEBUG")

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
    set(CMAKE_EXE_LINKER_FLAGS_DIST "")

    target_compile_definitions(Engine PRIVATE
        RK_PLATFORM_LINUX
        RK_BUILD_DLL
        RK_ENGINE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/Engine\"
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Required MS extension for compiling dxcapi and suppress nullability. Enables time-trace to track compilation timers.
    target_compile_options(Engine PRIVATE -fms-extensions -Wno-nullability-completeness -ftime-trace)
endif()

# Configuration-specific defines
foreach(CONFIG_TYPE IN ITEMS Debug Release Dist)
    string(TOUPPER ${CONFIG_TYPE} CONFIG_NAME_UPPER)
    if(${CONFIG_TYPE} STREQUAL "Debug")
        target_compile_definitions(Engine PRIVATE RK_DEBUG)
    elseif(${CONFIG_TYPE} STREQUAL "Release")
        target_compile_definitions(Engine PRIVATE RK_RELEASE)
    elseif(${CONFIG_TYPE} STREQUAL "Dist")
        target_compile_definitions(Engine PRIVATE RK_DIST)
    endif()
endforeach()

message("[ROCKET] Using ${CMAKE_CXX_COMPILER_ID} as CMake compiler")