cmake_minimum_required(VERSION 3.20)

set(target_name pbr)
project(${target_name})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

file(GLOB_RECURSE src_code LIST_DIRECTORIES false
        src/*.hpp src/*.h src/*.cpp src/*.c )


add_executable(${target_name} ${src_code})

add_subdirectory(libs)
target_link_libraries(${target_name} third_libs)

target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(${target_name} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
target_compile_definitions(${target_name} PRIVATE CURRENT_WORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
