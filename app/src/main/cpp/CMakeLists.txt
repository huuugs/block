cmake_minimum_required(VERSION 3.22)
project(BlockEater C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Raylib prebuilt library (will be created by CI during build)
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

# Include raylib headers (will be created by CI during build)
set(RAYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/raylib)
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${RAYLIB_INCLUDE_DIR}
    ${RAYLIB_INCLUDE_DIR}/external
)

# Include raygui (header-only library)
include_directories(${CMAKE_SOURCE_DIR}/raygui/src)

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
    camera.cpp
    raygui_impl.cpp
)

# Create shared library
add_library(main SHARED ${GAME_SOURCES})

# Link raylib static library and native app glue
# Use --whole-archive to ensure android_main from raylib is included
target_link_libraries(main
    -Wl,--whole-archive
    ${RAYLIB_DIR}/libraylib.a
    -Wl,--no-whole-archive
    android
    log
    GLESv3
    EGL
    OpenSLES
)

# Compiler flags
target_compile_options(main PRIVATE
    -O3
    -ffast-math
)

# Platform-specific definitions
target_compile_definitions(main PRIVATE
    PLATFORM_ANDROID
    GRAPHIC_API_OPENGL_ES_3
    SUPPORT_MODULE_RTEXT
    SUPPORT_MODULE_RAUDIO
)
