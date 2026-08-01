file(GLOB_RECURSE ENGINE_SRC
    src/*.cpp
)

add_library(handfx SHARED
    ${ENGINE_SRC}
)

target_include_directories(handfx
    PUBLIC
        include
)
