#!/bin/bash
set -e

echo "=== Building with Gradle NDK ==="

# Create stub raylib libraries (for linking)
mkdir -p raylib/lib/android/arm64-v8a
mkdir -p raylib/lib/android/armeabi-v7a

# Create a simple stub library for now
echo "Creating stub libraries..."
cat > /tmp/stub.c << 'STUB'
void raylib_stub() {}
int _raylib() { return 0; }
STUB

# Use system compiler to create minimal archives
gcc -c /tmp/stub.c -o /tmp/stub.o
ar rcs raylib/lib/android/arm64-v8a/libraylib.a /tmp/stub.o 2>/dev/null || touch raylib/lib/android/arm64-v8a/libraylib.a
cp raylib/lib/android/arm64-v8a/libraylib.a raylib/lib/android/armeabi-v7a/libraylib.a

echo "sdk.dir=/opt/android-sdk" > local.properties
echo "ndk.dir=$PWD/android-ndk" >> local.properties

# Build with Gradle (it will compile raylib and game together)
chmod +x gradlew 2>/dev/null || true
./gradlew assembleDebug 2>&1 | tail -40

ls -la app/build/outputs/apk/debug/*.apk 2>/dev/null || echo "APK not found"
