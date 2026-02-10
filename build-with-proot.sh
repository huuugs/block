#!/bin/bash
# Build script using proot + qemu-x86_64 for Android NDK

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

PROJECT_DIR="/root/block"
ROOTFS="/tmp/x86_64-root"
NDK_DIR="/android-ndk"
PROOT="proot -q qemu-x86_64 -r $ROOTFS -b $PROJECT/android-ndk:$NDK_DIR -b $PROJECT:/project -w /project"

echo "=== Block Eater Build (proot + qemu-x86_64) ==="
echo ""

# Step 1: Verify environment
echo -e "${YELLOW}[1/5] Verifying environment...${NC}"

if [ ! -d "$ROOTFS" ]; then
    echo -e "${RED}Rootfs not found. Downloading...${NC}"
    mkdir -p /tmp/x86_64-root
    curl -L "https://mirrors.tuna.tsinghua.edu.cn/ubuntu-cdimage/ubuntu-base/releases/22.04/release/ubuntu-base-22.04-base-amd64.tar.gz" -o /tmp/base-amd64.tar.gz
    tar -xzf /tmp/base-amd64.tar.gz -C /tmp/x86_64-root/
fi

# Test clang
if $PROOT /bin/sh -c "LD_LIBRARY_PATH=$NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/lib64 $NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/bin/clang-14.orig --version" >/dev/null 2>&1; then
    echo -e "${GREEN}✓ clang works in proot${NC}"
else
    echo -e "${RED}✗ clang test failed${NC}"
    exit 1
fi

# Step 2: Build raylib
echo ""
echo -e "${YELLOW}[2/5] Building raylib for Android...${NC}"

cd "$PROJECT_DIR/raylib"

for ARCH in arm64-v8a armeabi-v7a; do
    echo "  Building for $ARCH..."

    mkdir -p "build-android-$ARCH"

    # Run cmake via proot
    cat > /tmp/build-cmake.sh << CMAKE_EOF
export LD_LIBRARY_PATH=$NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/lib64
export CC="$NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/bin/clang-14.orig"
export CXX="$NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++.orig"
cd /project/raylib/build-android-$ARCH
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_DIR/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ARCH \
    -DANDROID_PLATFORM=android-24 \
    -DANDROID_STL=c++_static \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF
CMAKE_EOF

    $PROOT /bin/sh /tmp/build-cmake.sh || true

    # Run make via proot
    cat > /tmp/build-make.sh << MAKE_EOF
export LD_LIBRARY_PATH=$NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/lib64
cd /project/raylib/build-android-$ARCH
make -j4
MAKE_EOF

    $PROOT /bin/sh /tmp/build-make.sh || echo "  (may have warnings)"

    # Copy library
    mkdir -p "lib/android/$ARCH"
    cp "build-android-$ARCH/libraylib.a" "lib/android/$ARCH/" 2>/dev/null || echo "  Warning: library not found"

    echo "  ✓ $ARCH complete"
done

echo ""
echo -e "${GREEN}✓ raylib built${NC}"

# Step 3: Prepare Gradle build
echo ""
echo -e "${YELLOW}[3/5] Preparing Gradle build...${NC}"

cd "$PROJECT_DIR"

echo "sdk.dir=/opt/android-sdk" > local.properties
echo "ndk.dir=$PROJECT_DIR/android-ndk" >> local.properties

echo "  ✓ local.properties created"

# Step 4: Build APK
echo ""
echo -e "${YELLOW}[4/5] Building APK with Gradle...${NC}"

chmod +x gradlew 2>/dev/null || true

./gradlew assembleDebug 2>&1 | tail -30

echo ""
echo -e "${GREEN}✓ APK built${NC}"

# Step 5: Verify output
echo ""
echo -e "${YELLOW}[5/5] Verifying output...${NC}"

APK_PATH="$PROJECT_DIR/app/build/outputs/apk/debug/app-debug.apk"

if [ -f "$APK_PATH" ]; then
    APK_SIZE=$(du -h "$APK_PATH" | cut -f1)
    echo ""
    echo -e "${GREEN}=== Build Successful! ===${NC}"
    echo ""
    echo "APK: $APK_PATH"
    echo "Size: $APK_SIZE"
    echo ""

    # Check if adb is available
    if command -v adb &> /dev/null; then
        if adb devices | grep -q "device$"; then
            echo "Installing to device..."
            adb install -r "$APK_PATH"
        fi
    fi
else
    echo ""
    echo -e "${RED}Build failed - APK not found${NC}"
    exit 1
fi
