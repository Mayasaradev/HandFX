cmake_minimum_required(VERSION 3.22)

project(HandFX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    cpp/main.cpp
    cpp/engine/engine.cpp
    cpp/camera/camera.cpp
    cpp/renderer/renderer.cpp
    cpp/ai/inference.cpp
    cpp/geometry/geometry.cpp
    cpp/math/vector.cpp
    cpp/utils/logger.cpp
)
add_executable(HandFX ${SOURCES})
target_include_directories(HandFX PRIVATE cpp)
  
