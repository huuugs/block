# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Block Eater is a raylib-based Android game written in C++17. It features a block-eating gameplay mechanic with multiple game modes, procedural audio, touch controls, and multi-language support (English/Chinese).

## Build Commands

### Local Build (requires Android NDK)
```bash
# Build APK
./gradlew assembleDebug --no-daemon

# Clean build
./gradlew clean assembleDebug --no-daemon
```

### GitHub Actions
The project uses GitHub Actions for CI/CD. Push to `main` branch to build automatically.

**Required environment:**
- JDK 17
- Android NDK 25.2.9519653
- Gradle 8.5
- CMake 3.22+

## Architecture

### Game State Machine
The `Game` class (`game.h/cpp`) manages the entire game through a state machine:
- `GameState::MENU` - Main menu with mode selection
- `GameState::PLAYING` - Active gameplay
- `GameState::PAUSED` - Pause menu
- `GameState::GAME_OVER` - Game over screen
- `GameState::LEVEL_SELECT` - Level selection
- `GameState::SETTINGS` - Settings (language, theme, controls)

When switching states, always call `ui->resetTransition()` to prevent screen flash.

### Code Structure
All game code lives in `app/src/main/cpp/`:
- `game.h/cpp` - Main game class, state machine, game loop, entity management
- `player.h/cpp` - Player entity with 6 progression levels
- `enemy.h/cpp` - Four enemy types (Floating, Chasing, Stationary, Bouncing)
- `ui.h/cpp` - Menu system, HUD, multi-language support, themes
- `audio.h/cpp` - Procedural 8-bit sound generation
- `modes.h/cpp` - Game mode logic (Endless, Level, Time Challenge)
- `controls.h/cpp` - Virtual joystick and touch-follow input
- `particles.h/cpp` - Visual effects
- `assets.h/cpp` - Runtime asset generation and font loading

### Namespace
All game code uses `BlockEater::` namespace.

### Raylib Integration
- Raylib is built as a static library during CI with `c++_shared` STL
- Headers expected at: `app/src/main/cpp/include/raylib/`
- Static library at: `app/src/main/cpp/jniLibs/arm64-v8a/libraylib.a`
- Uses OpenGL ES 3.0 for Android rendering
- **Critical**: Raylib provides `android_main()` wrapper that calls the standard `main()` function

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

## Critical: Chinese Font Support (Known Issue)

**CURRENT PROBLEM**: Chinese text displays as question marks (???) in menus. This is an ongoing issue despite multiple attempts to fix it.

### Current Implementation
1. **Font file**: `app/src/main/cpp/fonts/zpix.ttf` (7MB Chinese pixel font)
2. **Font loading** (`assets.cpp`): Uses `LoadFontEx()` with explicit codepoint array containing only game-used Chinese characters (~100 characters)
3. **Text rendering** (`ui.cpp`): Custom button drawing uses `DrawTextEx()` with loaded font

### What Has Been Tried
- Loading full CJK range (0x4E00-0x9FFF) - caused memory issues
- Loading only game-specific characters - still shows ???
- Using raygui `GuiSetFont()` - raygui doesn't support custom fonts on Android
- Custom button drawing with `DrawTextEx()` - current approach, still broken

### Key Code Locations
- `assets.cpp:146-268` - `LoadExternalFont()` with Chinese codepoint array
- `ui.cpp:175-213` - Custom `drawButton()` using `DrawTextEx()`
- `ui.cpp:625-645` - `drawTextWithFont()` and `measureTextWithFont()` helpers

### Important Notes
- Font files are included via `build.gradle.kts` line 52: `assets.srcDirs("src/main/cpp/fonts")`
- On Android, fonts are accessed from assets, not filesystem
- `useCustomFont` flag in `UIManager` indicates if font loaded successfully
- Debug logging added to track font loading status

## Localization and Themes

### Language Support
The `UIManager` class supports multiple languages via the `Language` enum (ENGLISH, CHINESE). Use the `getText(english, chinese)` method for all translatable strings:
```cpp
const char* title = getText("START", "开始");
```

### Theme System
Five built-in themes are available (Blue, Dark, Green, Purple, Red). Themes define:
- `primary` - Primary UI color
- `secondary` - Secondary UI color
- `accent` - Accent/highlight color
- `background` - Background color
- `text` - Text color

Use `currentTheme->color` to reference theme colors in UI code.

## Critical: Native Library Configuration

The project uses Android NativeActivity framework with raylib. When modifying native code, keep these files synchronized:

1. **CMakeLists.txt** (`app/src/main/cpp/CMakeLists.txt`):
   - Library name MUST be `add_library(main SHARED ...)`
   - Uses `--whole-archive` linker flag to force inclusion of raylib's `android_main()`
   - Links with `c++_shared` STL (not `c++_static`)

2. **AndroidManifest.xml** (`app/src/main/AndroidManifest.xml`):
   - `<meta-data android:name="android.app.lib_name" android:value="main" />`

3. **MainActivity.java** (`app/src/main/java/com/blockeater/MainActivity.java`):
   - NativeActivity auto-loads the library via manifest metadata
   - No manual `System.loadLibrary()` call needed

4. **main.cpp** entry point:
   - Use standard `int main(int argc, char* argv[])` signature
   - Raylib provides its own `android_main()` wrapper that calls `main()`
   - Do NOT use `void android_main(android_app* app)` directly

5. **build.gradle.kts**:
   - Must specify `-DANDROID_STL=c++_shared` for C++ projects
   - Line 52: `assets.srcDirs("src/main/cpp/fonts")` includes fonts in APK

These must all match or the app will crash on startup with "Unable to find native library" errors.

## Touch Input Handling

Touch input is handled in each state's update function:
```cpp
if (GetTouchPointCount() > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 pos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
    // Check button bounds...
}
```

The game also has an in-game menu button (top-right corner, 70x35px) that pauses gameplay.

## UI Button System

**DO NOT use raygui buttons** - They don't support custom fonts on Android. Use `UIManager::drawButton()` instead:
```cpp
if (drawButton(x, y, width, height, getText("TEXT", "文本"))) {
    // Handle click
}
```

Custom button implementation:
- Supports touch and mouse input
- Uses `DrawTextEx()` for Chinese font support
- Handles hover states and disabled state
- Located in `ui.cpp:175-213`

## Control System

The game supports two control modes (toggleable in Settings):
- **Virtual Joystick**: Touch left half of screen to spawn joystick at touch position
- **Touch Follow**: Move toward touch point relative to player

Important implementation notes:
- Joystick only draws when active (finger touching) - see `controls.cpp:114-130`
- Joystick deactivates when `touchCount == 0` - see `controls.cpp:61-108`
- Control mode state is synced between Game, UIManager, and ControlSystem
