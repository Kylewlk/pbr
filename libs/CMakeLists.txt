cmake_minimum_required(VERSION 3.20)

set(target_name third_libs)
project(${target_name})

add_library(${target_name} INTERFACE)

target_sources(${target_name} INTERFACE Lib.cpp)

set_target_properties(${target_name}
        PROPERTIES FOLDER "${libs}"
)

target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# glfw
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(glfw)
target_link_libraries(${target_name} INTERFACE glfw)

# glad
target_sources(${target_name} INTERFACE glad/src/glad.c)
target_include_directories(${target_name} INTERFACE glad/include )

#imgui
file(GLOB imgui_src LIST_DIRECTORIES false imgui/*.h imgui/*.cpp)
list(APPEND imgui_src
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_glfw.h
        imgui/backends/imgui_impl_opengl3.cpp
        imgui/backends/imgui_impl_opengl3.h
)
target_sources(${target_name} INTERFACE ${imgui_src})
target_include_directories(${target_name} INTERFACE imgui)
