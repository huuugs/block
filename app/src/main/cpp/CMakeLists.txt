cmake_minimum_required(VERSION 3.22)
project(BlockEater C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find native_app_glue from NDK
# NDK r25+ has moved native_app_glue, try multiple locations
find_file(NATIVE_APP_GLUE
    NAMES native_app_glue.c
    PATHS
        ${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/*/sysroot/usr/src/android/native_app_glue/native_app_glue.c
        ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/native_app_glue.c
)

if(NOT NATIVE_APP_GLUE)
    message(WARNING "native_app_glue.c not found - trying alternative approach")
endif()

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

# Add native_app_glue if found
if(NATIVE_APP_GLUE)
    list(APPEND GAME_SOURCES ${NATIVE_APP_GLUE})
    message(STATUS "Adding native_app_glue.c: ${NATIVE_APP_GLUE}")
endif()

# Create shared library
add_library(main SHARED ${GAME_SOURCES})

# Link raylib static library and native app glue
target_link_libraries(main
    ${RAYLIB_DIR}/libraylib.a
    android
    log
    GLESv3
    EGL
    OpenSLES
)

# Linker flags
target_link_options(main PRIVATE
    -Wl,--exclude-libs,ALL
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
