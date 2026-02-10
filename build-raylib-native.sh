#!/bin/bash
# Build raylib using native ARM64 compiler with NDK sysroot

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

PROJECT_DIR="/root/block"
RAYLIB_DIR="${PROJECT_DIR}/raylib"
NDK_DIR="/root/block/android-ndk"

echo "=== Building raylib with Native Compiler ==="
echo ""

# Check raylib source
if [ ! -d "${RAYLIB_DIR}/src" ]; then
    echo -e "${RED}raylib source not found${NC}"
    exit 1
fi

cd "${RAYLIB_DIR}"

# Use a simpler approach - create the Android library manually
echo -e "${YELLOW}Creating raylib Android libraries...${NC}"

# For each architecture
for ARCH in arm64-v8a armeabi-v7a; do
    echo "Building for $ARCH..."

    mkdir -p "lib/android/$ARCH"

    # Create a stub library for now (this will need proper compilation)
    # In production, you would cross-compile or download prebuilt libs

    # For now, create empty lib files as placeholders
    # TODO: Actually compile raylib or use prebuilt libraries
    touch "lib/android/$ARCH/.placeholder"

    echo "  Created lib/android/$ARCH/"
done

echo ""
echo -e "${GREEN}Note: Full raylib compilation requires proper cross-compilation setup${NC}"
echo ""
echo "For a working build, you have these options:"
echo "  1. Build on x86_64 machine and copy libraries"
echo "  2. Use Docker with x86_64 container"
echo "  3. Download prebuilt raylib Android libraries"
echo ""
echo "Placeholder libraries created at:"
echo "  ${RAYLIB_DIR}/lib/android/"
echo ""
