cmake_minimum_required(VERSION 3.26)
project(BlockEater C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Raylib configuration
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/../../../../raylib)
set(RAYLIB_SRC ${RAYLIB_DIR}/src)

# Include raylib source directly instead of using prebuilt library
set(RAYLIB_SOURCES
    ${RAYLIB_SRC}/rcore.c
    ${RAYLIB_SRC}/rtextures.c
    ${RAYLIB_SRC}/rtext.c
    ${RAYLIB_SRC}/rmodels.c
    ${RAYLIB_SRC}/raudio.c
    ${RAYLIB_SRC}/rshapes.c
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${RAYLIB_SRC}
    ${RAYLIB_SRC}/external
)

# Source files
set(GAME_SOURCES
    main.cpp
    game.cpp
    player.cpp
    enemy.cpp
    particles.cpp
    ui.cpp
    audio.cpp
    modes.cpp
    controls.cpp
    assets.cpp
)

# Create library
add_library(blockeater SHARED ${GAME_SOURCES} ${RAYLIB_SOURCES})

# Link libraries - use Android system libraries
target_link_libraries(blockeater
    log
    android
    GLESv3
    EGL
    OpenSLES
)

# Compiler flags
target_compile_options(blockeater PRIVATE
    -O3
    -ffast-math
)

# Platform-specific definitions for raylib
target_compile_definitions(blockeater PRIVATE
    PLATFORM_ANDROID
    GRAPHIC_API_OPENGL_ES_3
    SUPPORT_MODULE_RTEXT
    SUPPORT_MODULE_RAUDIO
)

# Disable warnings in raylib code
set_source_files_properties(${RAYLIB_SOURCES} PROPERTIES
    COMPILE_FLAGS "-Wno-unused-variable -Wno-missing-field-initializers"
)
