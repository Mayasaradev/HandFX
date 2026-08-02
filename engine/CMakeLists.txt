cmake_minimum_required(VERSION 3.22)

project(HandFXEngine)

file(GLOB_RECURSE ENGINE_SRC
    src/*.cpp
)

add_library(handfx_engine STATIC
    ${ENGINE_SRC}
)

target_include_directories(handfx_engine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
