cmake_minimum_required(VERSION 3.22)

project(HandFX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(engine)

add_library(handfx SHARED
    app/src/main/cpp/native-lib.cpp
)

target_include_directories(handfx
    PRIVATE
    engine/include
)

target_link_libraries(handfx
    handfx_engine
    log
)
