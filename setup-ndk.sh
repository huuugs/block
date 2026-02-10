#!/bin/bash
# Download and setup Android NDK for Termux

set -e

NDK_VERSION="25.2.9519653"
NDK_DIR="/opt/android-ndk"
INSTALL_DIR="/root/block/ndk"

echo "=== Android NDK Setup ==="
echo ""

# Create install directory
mkdir -p "$INSTALL_DIR"

# Check if NDK already exists
if [ -f "$INSTALL_DIR/ndk-build" ]; then
    echo "NDK already installed at: $INSTALL_DIR"
    export ANDROID_NDK="$INSTALL_DIR"
    export NDK_HOME="$INSTALL_DIR"
    echo "ANDROID_NDK=$ANDROID_NDK"
    exit 0
fi

echo "Downloading Android NDK ${NDK_VERSION}..."

# Download NDK
NDK_URL="https://dl.google.com/android/repository/android-ndk-r${NDK_VERSION}-linux.zip"

if [ ! -f "/tmp/ndk-${NDK_VERSION}.zip" ]; then
    curl -L --max-time 600 "$NDK_URL" -o "/tmp/ndk-${NDK_VERSION}.zip" || {
        echo "Download failed, trying mirror..."
        # Try using a smaller NDK version or different source
        NDK_URL="https://github.com/android/ndk/releases/download/r25c/android-ndk-r25c-linux.zip"
        curl -L --max-time 600 "$NDK_URL" -o "/tmp/ndk-r25c-linux.zip" || {
            echo "Failed to download NDK"
            exit 1
        }
    }
fi

echo "Extracting NDK..."
unzip -q "/tmp/ndk-${NDK_VERSION}.zip" -d /tmp/ || unzip -q "/tmp/ndk-r25c-linux.zip" -d /tmp/

# Move to install directory
if [ -d "/tmp/android-ndk-r${NDK_VERSION}" ]; then
    mv "/tmp/android-ndk-r${NDK_VERSION}"/* "$INSTALL_DIR/"
elif [ -d "/tmp/android-ndk-r25c" ]; then
    mv "/tmp/android-ndk-r25c"/* "$INSTALL_DIR/"
else
    echo "NDK extraction failed"
    exit 1
fi

# Set permissions
chmod +x "$INSTALL_DIR/ndk-build"
chmod +x "$INSTALL_DIR/toolchains/llvm/prebuilt/linux-x86_64/bin/"*

echo ""
echo "NDK installed at: $INSTALL_DIR"
echo ""
echo "Add to your environment:"
echo "  export ANDROID_NDK=$INSTALL_DIR"
echo "  export NDK_HOME=$INSTALL_DIR"
