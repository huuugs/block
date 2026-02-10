cmake_minimum_required(VERSION 3.22)
project(BlockEater C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Raylib prebuilt library
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

# Check if raylib library exists
if(NOT EXISTS "${RAYLIB_DIR}/libraylib.a")
    message(FATAL_ERROR "Raylib library not found for ${ANDROID_ABI}. Expected: ${RAYLIB_DIR}/libraylib.a")
endif()

# Include raylib headers
set(RAYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/raylib)
if(EXISTS "${RAYLIB_INCLUDE_DIR}")
    include_directories(
        ${CMAKE_SOURCE_DIR}
        ${RAYLIB_INCLUDE_DIR}
        ${RAYLIB_INCLUDE_DIR}/external
    )
else()
    message(FATAL_ERROR "Raylib headers not found at: ${RAYLIB_INCLUDE_DIR}")
endif()

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

# Create shared library
add_library(main SHARED ${GAME_SOURCES})

# Link raylib static library
target_link_libraries(main
    ${RAYLIB_DIR}/libraylib.a
    log
    android
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
