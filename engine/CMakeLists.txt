cmake_minimum_required(VERSION 3.22)

project(HandFXEngine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE ENGINE_SRC
    src/*.cpp
)

add_library(engine STATIC
    ${ENGINE_SRC}
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
