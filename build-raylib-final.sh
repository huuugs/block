#!/bin/bash
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PROJECT="/root/block"
NDK="$PROJECT/android-ndk"
X86_ROOT="/tmp/x86-root"

export LD_LIBRARY_PATH="$X86_ROOT/lib:$X86_ROOT/lib64:/usr/x86_64-linux-gnu/lib"
export QEMU_LD_PREFIX="$X86_ROOT"
export PATH="/tmp/x86-root/usr/bin:$PATH"

export CC="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang-14.orig"
export CXX="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++.orig"
export AR="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar.orig"
export STRIP="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip.orig"

echo "=== Building raylib (qemu-x86_64) ==="

cd "$PROJECT/raylib"

for ARCH in arm64-v8a armeabi-v7a; do
    echo ""
    echo "Building $ARCH..."

    mkdir -p "build-android-$ARCH"
    cd "build-android-$ARCH"

    # Clean
    rm -rf *

    # CMake
    QEMU_LD_PREFIX="$X86_ROOT" qemu-x86_64 -L "$X86_ROOT" /tmp/x86-root/usr/bin/cmake .. \
        -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI=$ARCH \
        -DANDROID_PLATFORM=android-24 \
        -DANDROID_STL=c++_static \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_C_COMPILER="$CC" \
        -DCMAKE_CXX_COMPILER="$CXX" \
        -DCMAKE_AR="$AR" \
        -DCMAKE_STRIP="$STRIP" \
        2>&1 | tail -20

    # Build
    QEMU_LD_PREFIX="$X86_ROOT" qemu-x86_64 -L "$X86_ROOT" make -j4 2>&1 | tail -20

    # Copy
    mkdir -p "../lib/android/$ARCH"
    cp libraylib.a "../lib/android/$ARCH/" 2>/dev/null || echo "Library not found"

    cd ..
    echo "$ARCH done"
done

echo ""
echo "${GREEN}=== Raylib build complete ===${NC}"
ls -la "$PROJECT/raylib/lib/android/"*/libraylib.a
