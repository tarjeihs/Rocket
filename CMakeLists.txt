cmake_minimum_required(VERSION 3.14)
project(Rocket LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE ROCKET_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)

file(GLOB_RECURSE ROCKET_PRIVATE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
)

file(GLOB_RECURSE ROCKET_PRIVATE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
)

FetchContent_Declare(glfw_src)
FetchContent_Declare(imgui_src)
FetchContent_Declare(glm_src)
FetchContent_Declare(entt_src)
FetchContent_Declare(vma_src)
FetchContent_Declare(spdlog_src)

FetchContent_GetProperties(glfw_src)
FetchContent_GetProperties(imgui_src)
FetchContent_GetProperties(glm_src)
FetchContent_GetProperties(entt_src)
FetchContent_GetProperties(vma_src)
FetchContent_GetProperties(spdlog_src)

if(NOT glfw_src_POPULATED)
    FetchContent_Populate(glfw_src
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
endif()

if(NOT imgui_src_POPULATED)
    FetchContent_Populate(imgui_src
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.90.1
    )
endif()

if(NOT glm_src_POPULATED)
    FetchContent_Populate(glm_src
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
    )
endif()

if(NOT entt_src_POPULATED)
    FetchContent_Populate(entt_src
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG        v3.15.0
    )
endif()

if(NOT vma_src_POPULATED)
    FetchContent_Populate(vma_src
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG        v3.3.0
    )
endif()

if(NOT spdlog_src_POPULATED)
    FetchContent_Populate(spdlog_src
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG        v1.15.3
    )
endif()

add_library(glfw STATIC)
add_library(imgui STATIC)
add_library(glm::glm INTERFACE IMPORTED GLOBAL)
add_library(entt::entt INTERFACE IMPORTED GLOBAL)
add_library(vma::vma INTERFACE IMPORTED GLOBAL)
add_library(spdlog::spdlog INTERFACE IMPORTED GLOBAL)
add_executable(Rocket)

target_sources(glfw PRIVATE
    ${glfw_src_SOURCE_DIR}/src/context.c
    ${glfw_src_SOURCE_DIR}/src/init.c
    ${glfw_src_SOURCE_DIR}/src/input.c
    ${glfw_src_SOURCE_DIR}/src/monitor.c
    ${glfw_src_SOURCE_DIR}/src/platform.c 
    ${glfw_src_SOURCE_DIR}/src/posix_module.c
    ${glfw_src_SOURCE_DIR}/src/posix_poll.c
    ${glfw_src_SOURCE_DIR}/src/vulkan.c
    ${glfw_src_SOURCE_DIR}/src/window.c

    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:
    ${glfw_src_SOURCE_DIR}/src/x11_init.c
    ${glfw_src_SOURCE_DIR}/src/x11_monitor.c
    ${glfw_src_SOURCE_DIR}/src/x11_window.c
    ${glfw_src_SOURCE_DIR}/src/xkb_unicode.c
    ${glfw_src_SOURCE_DIR}/src/posix_time.c
    ${glfw_src_SOURCE_DIR}/src/posix_thread.c
    ${glfw_src_SOURCE_DIR}/src/glx_context.c
    ${glfw_src_SOURCE_DIR}/src/egl_context.c
    ${glfw_src_SOURCE_DIR}/src/osmesa_context.c
    ${glfw_src_SOURCE_DIR}/src/linux_joystick.c 
    >

    $<$<BOOL:${WIN32}>:
    ${glfw_src_SOURCE_DIR}/src/win32_init.c
    ${glfw_src_SOURCE_DIR}/src/win32_module.c
    ${glfw_src_SOURCE_DIR}/src/win32_monitor.c
    ${glfw_src_SOURCE_DIR}/src/win32_window.c
    ${glfw_src_SOURCE_DIR}/src/win32_time.c
    ${glfw_src_SOURCE_DIR}/src/win32_thread.c
    ${glfw_src_SOURCE_DIR}/src/win32_joystick.c
    >

    # “Null” stub backend (required by platform.c)
    ${glfw_src_SOURCE_DIR}/src/null_init.c 
    ${glfw_src_SOURCE_DIR}/src/null_monitor.c
    ${glfw_src_SOURCE_DIR}/src/null_window.c
    ${glfw_src_SOURCE_DIR}/src/null_joystick.c
)

target_sources(imgui PRIVATE
    ${imgui_src_SOURCE_DIR}/imgui.cpp
    ${imgui_src_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_src_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_src_SOURCE_DIR}/imgui_widgets.cpp
)

target_sources(Rocket PRIVATE
    ${ROCKET_PRIVATE_SOURCES}
    ${ROCKET_PRIVATE_HEADERS}
    ${ROCKET_PUBLIC_HEADERS}
)

set_target_properties(glfw PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${glfw_src_SOURCE_DIR}/include"
)
set_target_properties(imgui PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${imgui_src_SOURCE_DIR}"
)
set_target_properties(glm::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${glm_src_SOURCE_DIR}"
)
set_target_properties(entt::entt PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${entt_src_SOURCE_DIR}/single_include"
)
set_target_properties(vma::vma PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${vma_src_SOURCE_DIR}/include"
)
set_target_properties(spdlog::spdlog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${spdlog_src_SOURCE_DIR}/include"
)
target_include_directories(Rocket
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Source
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(Rocket PRIVATE
    #GLFW_INCLUDE_VULKAN
    RK_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    RK_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/Source\"
)

target_link_libraries(Rocket PRIVATE 
    glfw 
    imgui 
    vma::vma 
    glm::glm 
    entt::entt 
    spdlog::spdlog
)

target_precompile_headers(Rocket PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/RocketPCH.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(Rocket PRIVATE RK_PLATFORM_LINUX)
    target_include_directories(Rocket PRIVATE $ENV{VULKAN_SDK}/include)
    target_link_directories(Rocket PRIVATE $ENV{VULKAN_SDK}/lib)
    target_link_libraries(Rocket PRIVATE libvulkan.so)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Rocket PRIVATE RK_PLATFORM_WINDOWS)
    target_include_directories(Rocket PRIVATE $ENV{VULKAN_SDK}/Include)
    target_link_directories(Rocket PRIVATE $ENV{VULKAN_SDK}/Lib)
    target_link_libraries(Rocket PRIVATE vulkan-1)
endif()

add_custom_command(
    TARGET Rocket POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_BINARY_DIR}/../compile_commands.json
)
