#!/bin/bash
# Build script using Docker for x86_64 environment

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=== Block Eater Docker Build ==="
echo ""

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    echo -e "${YELLOW}Docker not available${NC}"
    echo ""
    echo "Setting up build environment..."

    # Alternative: Use a chroot or container approach
    if command -v proot &> /dev/null; then
        echo "Using proot for x86_64 emulation..."
        # This would require downloading an x86_64 rootfs
    fi

    echo ""
    echo "Recommended options:"
    echo "  1. Install Docker"
    echo "  2. Use GitHub Actions (push to GitHub)"
    echo "  3. Use x86_64 VPS/cloud machine"
    echo ""
    exit 1
fi

echo -e "${GREEN}Docker found!${NC}"

# Create Dockerfile
cat > /tmp/Dockerfile.blockeater << 'EOF'
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    unzip \
    openjdk-17-jdk \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /project

# Copy project
COPY . /project/

# Download and setup NDK
RUN mkdir -p /opt/android-ndk && \
    cd /opt && \
    wget -q https://dl.google.com/android/repository/android-ndk-r25c-linux.zip && \
    unzip -q android-ndk-r25c-linux.zip && \
    rm android-ndk-r25c-linux.zip

ENV ANDROID_NDK=/opt/android-ndk

# Build raylib
WORKDIR /opt/raylib-build
RUN curl -L https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz | tar -xz && \
    cd raylib-5.0 && \
    for arch in arm64-v8a armeabi-v7a; do \
        mkdir -p build-android-$arch && \
        cd build-android-$arch && \
        cmake .. \
            -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
            -DANDROID_ABI=$arch \
            -DANDROID_PLATFORM=android-24 \
            -DANDROID_STL=c++_static \
            -DBUILD_SHARED_LIBS=OFF && \
        cmake --build . && \
        mkdir -p /project/raylib/lib/android/$arch && \
        cp libraylib.a /project/raylib/lib/android/$arch/ && \
        cd .. ; \
    done

# Build APK
WORKDIR /project
ENV ANDROID_SDK_ROOT=/opt/android-sdk
ENV GRADLE_USER_HOME=/project/.gradle

RUN echo "sdk.dir=$ANDROID_SDK_ROOT" > local.properties

RUN gradle assembleDebug || ./gradlew assembleDebug

CMD ["bash"]
EOF

echo "Dockerfile created"

# Build and run
echo ""
echo "Building Docker image..."
docker build -f /tmp/Dockerfile.blockeater -t blockeater-build .

echo ""
echo -e "${GREEN}Build complete!${NC}"
echo "APK location: /root/block/app/build/outputs/apk/debug/app-debug.apk"
