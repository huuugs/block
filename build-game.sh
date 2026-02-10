#!/bin/bash
# Block Eater Game Build Script for Termux/Android

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
RAYLIB_VERSION="5.0"
PROJECT_DIR="/root/block"
BUILD_DIR="${PROJECT_DIR}/app/build"
RAYLIB_DIR="${PROJECT_DIR}/raylib"
NDK_VERSION="25.2.9519653"
API_LEVEL="24"

echo -e "${GREEN}=== Block Eater - Raylib Android Build ===${NC}"
echo ""

# Step 1: Download raylib source
download_raylib() {
    echo -e "${BLUE}[1/6] Downloading raylib source...${NC}"

    if [ -d "${RAYLIB_DIR}/src" ]; then
        echo "raylib source already exists, skipping download"
        return
    fi

    mkdir -p "${RAYLIB_DIR}"

    # Download using git or tarball
    if command -v git &> /dev/null; then
        git clone --depth 1 --branch ${RAYLIB_VERSION} https://github.com/raysan5/raylib.git "${RAYLIB_DIR}" 2>/dev/null || {
            # Fallback to tarball
            download_raylib_tarball
        }
    else
        download_raylib_tarball
    fi

    echo -e "${GREEN}raylib source downloaded${NC}"
}

download_raylib_tarball() {
    local url="https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz"
    local temp_file="/tmp/raylib-${RAYLIB_VERSION}.tar.gz"

    echo "Downloading tarball..."
    curl -L --max-time 120 "$url" -o "$temp_file" || {
        echo -e "${RED}Failed to download raylib${NC}"
        return 1
    }

    tar -xzf "$temp_file" -C /tmp
    rm -f "$temp_file"
    mv /tmp/raylib-${RAYLIB_VERSION}/* "${RAYLIB_DIR}/"
    rmdir /tmp/raylib-${RAYLIB_VERSION} 2>/dev/null || true
}

# Step 2: Build raylib for Android
build_raylib() {
    echo -e "${BLUE}[2/6] Building raylib for Android...${NC}"

    cd "${RAYLIB_DIR}"

    # Create CMakeLists for raylib Android build
    cat > CMakeLists.txt << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.20)
project(raylib C)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Platform specific
if(ANDROID)
    set(PLATFORM "ANDROID")
    set(GRAPHIC GRAPHIC_API_OPENGL_ES_3)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM "DRM")
    set(GRAPHIC GRAPHIC_API_OPENGL_ES_3)
elseif(WIN32)
    set(PLATFORM "DESKTOP")
    set(GRAPHIC GRAPHIC_API_OPENGL_33)
else()
    set(PLATFORM "DESKTOP")
    set(GRAPHIC GRAPHIC_API_OPENGL_33)
endif()

# Raylib source files
set(RAYLIB_SRC
    src/raylib.c
    src/rcore.c
    src/rtextures.c
    src/rtext.c
    src/rmodels.c
    src/raudio.c
    src/rshapes.c
)

if(ANDROID)
    list(APPEND RAYLIB_SRC src/raudio.c src/rcore.c)
endif()

# Create library
add_library(raylib STATIC ${RAYLIB_SRC})

# Include directories
target_include_directories(raylib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/external
)

# Platform-specific definitions
if(ANDROID)
    target_compile_definitions(raylib PUBLIC
        ANDROID
        PLATFORM_ANDROID
        SUPPORT_MODULE_RTEXT
        SUPPORT_MODULE_RAUDIO
    )
endif()

# Link libraries
if(ANDROID)
    target_link_libraries(raylib PUBLIC
        log
        android
        EGL
        GLESv3
        OpenSLES
    )
endif()
CMAKE_EOF

    # Build for each architecture
    for ARCH in arm64-v8a armeabi-v7a; do
        echo "Building raylib for $ARCH..."

        mkdir -p build-android-$ARCH
        cd build-android-$ARCH

        cmake .. \
            -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
            -DANDROID_ABI=$ARCH \
            -DANDROID_PLATFORM=android-$API_LEVEL \
            -DANDROID_STL=c++_static \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_SHARED_LIBS=OFF

        cmake --build . --config Release

        # Copy library to expected location
        mkdir -p ../lib/android/$ARCH
        cp libraylib.a ../lib/android/$ARCH/

        cd ..
    done

    echo -e "${GREEN}raylib built for Android${NC}"

    cd "${PROJECT_DIR}"
}

# Step 3: Create gradle wrapper files
setup_gradle() {
    echo -e "${BLUE}[3/6] Setting up Gradle...${NC}"

    cd "${PROJECT_DIR}"

    # Check if gradle wrapper jar exists, if not try to use system gradle
    if [ ! -f "gradle/wrapper/gradle-wrapper.jar" ]; then
        echo "Gradle wrapper jar not found, using system gradle"

        # Create a simple gradlew that uses system gradle
        cat > gradlew << 'GRADLEW_EOF'
#!/bin/bash
# Simple gradle wrapper that uses system gradle

if command -v gradle &> /dev/null; then
    # Forward all arguments to gradle
    gradle "$@"
else
    echo "Error: gradle not found in PATH"
    echo "Please install gradle or download gradle wrapper"
    exit 1
fi
GRADLEW_EOF
        chmod +x gradlew
    fi

    echo -e "${GREEN}Gradle ready${NC}"
}

# Step 4: Build the APK
build_apk() {
    echo -e "${BLUE}[4/6] Building APK...${NC}"

    cd "${PROJECT_DIR}"

    # Clean previous builds
    rm -rf app/build

    # Build debug APK
    chmod +x gradlew
    ./gradlew assembleDebug --stacktrace || {
        echo -e "${RED}Build failed${NC}"
        return 1
    }

    echo -e "${GREEN}APK built successfully${NC}"
    echo "Location: ${PROJECT_DIR}/app/build/outputs/apk/debug/app-debug.apk"
}

# Step 5: Install to device
install_apk() {
    echo -e "${BLUE}[5/6] Installing to device...${NC}"

    if ! command -v adb &> /dev/null; then
        echo -e "${YELLOW}adb not found${NC}"
        echo "Install Android SDK platform-tools or copy APK manually:"
        echo "  ${PROJECT_DIR}/app/build/outputs/apk/debug/app-debug.apk"
        return
    fi

    if adb devices | grep -q "device$"; then
        adb install -r "${PROJECT_DIR}/app/build/outputs/apk/debug/app-debug.apk"
        echo -e "${GREEN}Installed to device${NC}"
    else
        echo -e "${YELLOW}No device connected${NC}"
    fi
}

# Step 6: Verify build
verify_build() {
    echo -e "${BLUE}[6/6] Verifying build...${NC}"

    if [ -f "${PROJECT_DIR}/app/build/outputs/apk/debug/app-debug.apk" ]; then
        local size=$(du -h "${PROJECT_DIR}/app/build/outputs/apk/debug/app-debug.apk" | cut -f1)
        echo -e "${GREEN}Build successful!${NC}"
        echo "APK size: $size"
        return 0
    else
        echo -e "${RED}Build verification failed${NC}"
        return 1
    fi
}

# Quick build (skip raylib download if already done)
quick_build() {
    build_apk && verify_build
}

# Main execution
main() {
    case "${1:-all}" in
        setup)
            download_raylib
            build_raylib
            setup_gradle
            ;;
        raylib)
            download_raylib
            build_raylib
            ;;
        build)
            setup_gradle
            build_apk
            ;;
        install)
            install_apk
            ;;
        quick)
            quick_build
            ;;
        all)
            download_raylib
            build_raylib
            setup_gradle
            build_apk
            verify_build
            echo ""
            echo -e "${GREEN}=== Build Complete ===${NC}"
            echo ""
            echo "Install to device? (y/n)"
            read -r answer
            if [ "$answer" = "y" ]; then
                install_apk
            fi
            ;;
        clean)
            echo "Cleaning..."
            rm -rf "${BUILD_DIR}"
            rm -rf "${PROJECT_DIR}/.gradle"
            rm -rf "${PROJECT_DIR}/app/.cxx"
            rm -rf "${RAYLIB_DIR}/build-android-"*
            echo -e "${GREEN}Clean complete${NC}"
            ;;
        *)
            cat << USAGE
Usage: $0 [command]

Commands:
  setup   - Download and build raylib for Android
  raylib  - Only build raylib
  build   - Build APK (requires raylib already built)
  install - Install APK to connected device
  quick   - Quick build (skip raylib download)
  all     - Full setup and build
  clean   - Clean build files

Environment:
  NDK_VERSION=${NDK_VERSION}
  API_LEVEL=${API_LEVEL}
USAGE
            exit 1
            ;;
    esac
}

main "$@"
