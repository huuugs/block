# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Block Eater is a raylib-based Android game written in C++17. It features a block-eating gameplay mechanic with multiple game modes, procedural audio, and touch controls.

## Build Commands

### Local Build (requires Android NDK)
```bash
# Build APK
./gradlew assembleDebug --no-daemon

# Clean build
./gradlew clean assembleDebug --no-daemon
```

### GitHub Actions
The project uses GitHub Actions for CI/CD. Push to `main` branch or manually trigger via `workflow_dispatch` to build.

**Required environment:**
- JDK 17
- Android NDK 25.2.9519653
- Gradle 8.5
- CMake 3.22+

## Architecture

### Code Structure
All game code lives in `app/src/main/cpp/`:
- `game.h/cpp` - Main game class, state machine, and game loop
- `player.h/cpp` - Player entity with 6 progression levels
- `enemy.h/cpp` - Four enemy types (Floating, Chasing, Stationary, Bouncing)
- `ui.h/cpp` - Menu system and HUD
- `audio.h/cpp` - Procedural 8-bit sound generation
- `modes.h/cpp` - Game mode logic (Endless, Level, Time Challenge)
- `controls.h/cpp` - Virtual joystick and touch-follow input
- `particles.h/cpp` - Visual effects
- `assets.h/cpp` - Runtime asset generation

### Namespace
All game code uses `BlockEater::` namespace.

### Raylib Integration
- Raylib is built as a static library during CI
- Headers expected at: `app/src/main/cpp/include/raylib/`
- Static library at: `app/src/main/cpp/jniLibs/arm64-v8a/libraylib.a`
- Uses OpenGL ES 3.0 for Android rendering

### Android-Only Build
The project is configured for `arm64-v8a` only. The 32-bit build is disabled due to NEON compilation issues in raylib 5.0.

## Key Constants
- Screen: 1280x720 (landscape)
- Target FPS: 60
- Min Android API: 24 (Android 7.0)

## C++ Notes
- Uses C++17 features
- Narrowing conversions in initializer lists require explicit casts (e.g., `static_cast<unsigned int>()`)
- raylib headers must be included as `#include "raylib.h"` (not `<raylib.h>`)

## Critical: Native Library Configuration

The project uses Android NativeActivity framework. When modifying native code, keep these three files synchronized:

1. **CMakeLists.txt** (`app/src/main/cpp/CMakeLists.txt`):
   - Library name MUST be `add_library(main SHARED ...)`
   - All target references must use `main` (not the app name)

2. **AndroidManifest.xml** (`app/src/main/AndroidManifest.xml`):
   - `<meta-data android:name="android.app.lib_name" android:value="main" />`

3. **MainActivity.java** (`app/src/main/java/com/blockeater/MainActivity.java`):
   - `System.loadLibrary("main")` in static initializer

4. **main.cpp** entry point:
   - Must use `void android_main(android_app* app)` signature (NOT `int main()`)
   - NativeActivity framework calls this function automatically

These four must match or the app will crash on startup with "Unable to find native library" errors.
