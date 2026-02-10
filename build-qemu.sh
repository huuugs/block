#!/bin/bash
# Build raylib and Block Eater using qemu-x86_64

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PROJECT="/root/block"
NDK="$PROJECT/android-ndk"
X86_ROOT="/tmp/x86-root"

echo "=== Block Eater Build (qemu-x86_64) ==="
echo ""

# Setup environment
export LD_LIBRARY_PATH="$X86_ROOT/lib64:/usr/x86_64-linux-gnu/lib64"
export QEMU_LD_PREFIX="$X86_ROOT"
export CC="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang-14.orig"
export CXX="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++.orig"
export AR="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar.orig"
export STRIP="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip.orig"

# qemu wrapper function
run_x86() {
    qemu-x86_64 -L "$X86_ROOT" "$@"
}

# Step 1: Build raylib
echo -e "${YELLOW}[1/3] Building raylib for Android...${NC}"

cd "$PROJECT/raylib"

for ARCH in arm64-v8a armeabi-v7a; do
    echo "  Building for $ARCH..."

    mkdir -p "build-android-$ARCH"
    cd "build-android-$ARCH"

    # Run cmake
    run_x86 cmake .. \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI=$ARCH \
        -DANDROID_PLATFORM=android-24 \
        -DANDROID_STL=c++_static \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_C_COMPILER="$CC" \
        -DCMAKE_CXX_COMPILER="$CXX" \
        -DCMAKE_AR="$AR" \
        -DCMAKE_STRIP="$STRIP"

    # Build
    run_x86 make -j4

    # Copy library
    mkdir -p "../lib/android/$ARCH"
    cp libraylib.a "../lib/android/$ARCH/"

    cd ..
    echo "  ✓ $ARCH complete"
done

echo ""
echo -e "${GREEN}✓ raylib built${NC}"

# Step 2: Build APK
echo ""
echo -e "${YELLOW}[2/3] Building APK with Gradle...${NC}"

cd "$PROJECT"

echo "sdk.dir=/opt/android-sdk" > local.properties
echo "ndk.dir=$NDK" >> local.properties

chmod +x gradlew 2>/dev/null || true
./gradlew assembleDebug 2>&1 | tail -30

echo ""
echo -e "${GREEN}✓ APK built${NC}"

# Step 3: Verify
echo ""
echo -e "${YELLOW}[3/3] Verifying...${NC}"

APK="$PROJECT/app/build/outputs/apk/debug/app-debug.apk"

if [ -f "$APK" ]; then
    SIZE=$(du -h "$APK" | cut -f1)
    echo ""
    echo -e "${GREEN}=== Build Successful! ===${NC}"
    echo ""
    echo "APK: $APK"
    echo "Size: $SIZE"
    echo ""

    if command -v adb &> /dev/null && adb devices | grep -q "device"; then
        echo "Installing to device..."
        adb install -r "$APK"
    fi
else
    echo -e "${RED}Build failed${NC}"
    exit 1
fi
