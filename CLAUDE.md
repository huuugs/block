# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Block Eater is a raylib-based Android game written in C++17. It features a block-eating gameplay mechanic with multiple game modes, procedural audio, touch controls, multi-language support (English/Chinese), a user system with persistent statistics, and physics-based movement.

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
- `GameState::USER_MENU` - User system menu

When switching states, always call `ui->resetTransition()` to prevent screen flash.

### Code Structure
All game code lives in `app/src/main/cpp/`:
- `game.h/cpp` - Main game class, state machine, game loop, entity management
- `player.h/cpp` - Player entity with 15 progression levels, physics-based movement
- `enemy.h/cpp` - Four enemy types (Floating, Chasing, Stationary, Bouncing) with physics
- `bullet.h/cpp` - Projectile system for shooting skill
- `ui.h/cpp` - Menu system, HUD, multi-language support, themes, log viewer
- `audio.h/cpp` - Procedural 8-bit sound generation
- `modes.h/cpp` - Game mode logic (Endless, Level, Time Challenge)
- `controls.h/cpp` - Virtual joystick and touch-follow input
- `particles.h/cpp` - Visual effects
- `assets.h/cpp` - Runtime asset generation and font loading
- `userManager.h/cpp` - User system with statistics tracking
- `user.h/cpp` - User data structure
- `skills.h/cpp` - Skill system (4 skills with cooldowns)

### Namespace
All game code uses `BlockEater::` namespace.

### Physics System (Important!)
The game uses a physics-based movement system with rigid body collision:

**Player Physics:**
- Movement uses `applyForce()` and acceleration-based steering
- `applyJoystickInput()` applies force toward joystick direction
- `mass` is calculated from size (volume-based: `size * size * size`)
- Velocity and acceleration are managed per-frame
- Level affects combat stats (health, armor), not size

**Enemy Physics:**
- Same force-based movement as player
- `applyRigidBodyCollision()` handles bouncing between entities
- Collision response uses momentum conservation: `v1' = v1 - 2*m2/(m1+m2) * dot(v1-v2,n) * n`
- `applyBouncingDamage()` handles BOUNCING type's special high-damage collisions

**Critical: Vector2 Negation**
raylib's `Vector2` does NOT support unary minus operator. Always use:
```cpp
Vector2 negNormal = (Vector2){-normal.x, -normal.y};
```
Do NOT use `-normal` - this causes compilation errors.

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

## APK Signing Configuration

The project uses a **fixed signing keystore** stored in GitHub Secrets to ensure consistent APK signatures for upgrade installation.

### Local Development
For local builds, the signing config in `build.gradle.kts` expects:
- Keystore path: `~/.android/debug.keystore`
- Store password: `blockeater2024`
- Key alias: `blockeaterdebug`
- Key password: `blockeater2024`

### CI/CD Configuration
- GitHub Secret: `DEBUG_KEYSTORE_BASE64` (base64-encoded keystore)
- Workflow decodes keystore to `~/.android/debug.keystore` before building
- `build.gradle.kts` uses `blockeaterDebug` signingConfig (not `debug` to avoid conflict with Android Gradle Plugin's default)

**Important**: The signing config name is `blockeaterDebug`, not `debug`, because AGP automatically creates a `debug` signingConfig.

## Critical: Chinese Font Support

**SOLVED**: Chinese text displays correctly using `LoadFileData()` + `LoadFontFromMemory()` method.

### Working Implementation
On Android, `LoadFontEx()` cannot access assets directory directly. The working solution uses:

```cpp
// Load font file into memory (works on Android!)
unsigned char* fileData = LoadFileData(path, &fileSize);

// Load font from memory
const char* ext = (strstr(path, ".ttf") != nullptr) ? ".ttf" : ".otf";
pixelFont = LoadFontFromMemory(ext, fileData, fileSize, fontSize, codepoints, codepointCount);
UnloadFileData(fileData);
```

### Font Files (in `app/src/main/cpp/fonts/`)
- `zpix.ttf` - Primary Chinese pixel font (~7MB, ~3500 characters)
- `vonwaon_pixel_12px.ttf` - Alternative pixel font (fallback)

**Note**: Source Han Sans was removed to reduce APK size from ~30MB to ~7MB.

### Font Loading Process
1. `LoadCodepoints(allText, &codepointCount)` extracts unique characters from text
2. `LoadFileData()` reads font file into memory
3. `LoadFontFromMemory()` creates font from memory
4. `SetTextureFilter()` and `GenTextureMipmaps()` optimize rendering

### Chinese Character Set
The `allChineseText` string in `assets.cpp:241-405` contains ~3500 common Chinese characters organized by category:
- Game UI text, user system text
- Top 100 most common characters
- Common surnames and given names
- Verbs, adjectives, measure words
- Time expressions, directions, emotions
- Daily items and common phrases

### Key Code Locations
- `assets.cpp:234-500` - `LoadExternalFont()` with LoadFileData + LoadFontFromMemory
- `ui.cpp:70-158` - `init()` with SetTraceLogCallback for log synchronization
- `ui.cpp:814-834` - `drawTextWithFont()` and `measureTextWithFont()` helpers

### Important Notes
- Font files are included via `build.gradle.kts` line 64: `assets.srcDirs("src/main/cpp/fonts")`
- On Android, fonts are accessed from APK assets, not filesystem
- `SetTraceLogCallback` redirects all raylib logs to in-game log viewer
- `useCustomFont` flag in `UIManager` indicates if font loaded successfully

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
   - Line 64: `assets.srcDirs("src/main/cpp/fonts")` includes fonts in APK
   - Signing config named `blockeaterDebug` to avoid AGP conflict

These must all match or the app will crash on startup with "Unable to find native library" errors.

## Touch Input Handling

### Multitouch Best Practices
**CRITICAL**: Always iterate through ALL touch points when detecting UI interactions that need to work simultaneously with controls.

```cpp
// WRONG - Only checks first touch point
Vector2 pos = GetTouchPosition(0);
if (CheckCollisionPointRec(pos, buttonRect)) { ... }

// CORRECT - Checks all touch points
int touchCount = GetTouchPointCount();
for (int t = 0; t < touchCount; t++) {
    Vector2 pos = GetTouchPosition(t);
    if (CheckCollisionPointRec(pos, buttonRect)) { ... }
}
```

This is essential for:
- Skill buttons while using joystick (touch 0: joystick, touch 1: skills)
- Multiple simultaneous UI interactions
- Better mobile gameplay experience

### Single Touch Pattern
For menu interactions where only one touch is needed:
```cpp
if (GetTouchPointCount() > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    Vector2 pos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
    // Check button bounds...
}
```

The game has an in-game menu button (top-right corner, 70x35px) that pauses gameplay.

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

## User System

The game includes a user management system with persistent statistics:
- **5 user slots** - Support for up to 5 different user profiles
- **Statistics tracking** - High scores, play time, games played per mode
- **Persistent storage** - Saved to `user_data.dat` in binary format
- **Username support** - Chinese and English usernames supported

### User Data Structure
Each user tracks:
- `username` - Display name (up to 63 characters)
- `totalGamesPlayed`, `totalPlayTime`, `totalScore` - Aggregate stats
- `endlessStats`, `levelStats`, `timeChallengeStats` - Per-mode statistics
- `maxLevelUnlocked` - Progress tracking

### Key Code Locations
- `userManager.h/cpp` - User management and save/load
- `user.h/cpp` - User data structure
- `ui.cpp:978-1057` - `drawUserMenu()` for user selection UI
- `game.cpp:75-77` - User manager initialization

## Log Viewer System

The game has an in-game log viewer accessible from Settings > "View Logs". All raylib logs are automatically redirected to this viewer.

### Log Callback Setup
```cpp
SetTraceLogCallback([](int logLevel, const char* text, va_list args) -> void {
    switch (logLevel) {
        case LOG_INFO: UIManager::logInfo(text); break;
        case LOG_WARNING: UIManager::logWarning(text); break;
        case LOG_ERROR: UIManager::logError(text); break;
        default: UIManager::logInfo(text); break;
    }
});
```

### Log Buffer
- `MAX_LOG_ENTRIES = 50` - Circular buffer holds last 50 log entries
- `MAX_LOG_LENGTH = 256` - Each log entry up to 256 characters
- Located in `ui.cpp:25-28` - Static log buffer implementation
- Draw log viewer in `ui.cpp:890-935`

## Game Modes

### Endless Mode
- Continuous gameplay with increasing difficulty
- No time limit
- Score and level progression only

### Level Mode
- 10 predefined levels with specific objectives
- Some levels have time limits (check `levelDefinitions` in `modes.cpp`)
- Objectives: reach score, reach level, or survive for time

### Time Challenge
- 3-minute countdown timer
- Eating food blocks adds bonus time
- Goal: maximize score before time runs out

## Skill System

The player has 4 active skills with cooldowns (displayed in bottom-right corner):
1. **Rotate** (旋转) - 5s cooldown - Damage reduction and reflection
2. **Blink** (闪现) - 8s cooldown - Teleport 5x player size in facing direction
3. **Shoot** (射击) - 3s cooldown - Fire bullet costing 20 HP, deals 3x damage
4. **Shield** (护盾) - 15s cooldown - Arc-shaped shield that blocks enemies

### Important Implementation Notes
- Skills use energy from player's energy bar
- Each skill has independent cooldown
- Shield requires `shieldTimeLeft = shieldDuration` initialization (fixed in skills.cpp:161)
- Shield duration increases with player level (1-15 seconds based on level)
- See `skills.cpp:106-168` for skill activation logic

### CRITICAL: Multitouch Support for Skills
**FIXED**: Skills and direction can now be used simultaneously (commit 7373d38).

**The Problem**: Original code only checked `GetTouchPosition(0)`, which conflicted with joystick input.

**The Solution** - Iterate through ALL touch points (`game.cpp:290-347`):
```cpp
// Check each touch point for skill button clicks
for (int t = 0; t < touchCount; t++) {
    Vector2 pos = GetTouchPosition(t);
    // Check if this touch is on a skill button...
}
```

This allows:
- Touch point 0: Joystick control (left half of screen)
- Touch point 1+: Skill buttons (bottom-right corner)
- Simultaneous movement and skill activation

## Background and Assets

The game generates its own assets at runtime to reduce APK size:
- **Space background** - Procedurally generated with stars (`assets.cpp:149-232`)
- **Pixel blocks** - Generated with edge effects (`assets.cpp:58-105`)
- **Audio** - Procedural 8-bit chiptune sounds (`audio.cpp`)

Background texture is loaded in `game.cpp:51` and drawn with `drawBackground()`.

## Known Issues and Bugs Fixed

### Fixed: Shield Not Displaying
**Problem**: Skill 4 (shield) didn't appear when used
**Solution**: Added `shieldTimeLeft = shieldDuration;` in `skills.cpp:161`

### Fixed: Level Mode Instant Death
**Problem**: Entering level mode caused immediate game over
**Solution**: Fixed time limit loading - levels with `timeLimit = 0` have no time limit (game.cpp:679-694)

### Fixed: Chinese Font Not Loading
**Problem**: `LoadFontEx()` couldn't access Android assets
**Solution**: Use `LoadFileData()` + `LoadFontFromMemory()` pattern (see Font Support section)

### Fixed: Skills and Direction Cannot Respond Simultaneously
**Problem**: Using joystick prevented skill activation
**Solution**: Changed from checking `GetTouchPosition(0)` to iterating all touch points (game.cpp:291-347)

### Fixed: APK Signing Inconsistency
**Problem**: Each CI build used different keystore, preventing upgrade installation
**Solution**: Use fixed keystore stored in GitHub Secrets (`DEBUG_KEYSTORE_BASE64`)

### Fixed: Vector2 Unary Minus Compilation Error
**Problem**: Using `-normal` caused "invalid argument type 'Vector2' to unary expression"
**Solution**: Use manual negation `(Vector2){-normal.x, -normal.y}` instead

## Adding New Content

### Adding New Chinese Text
1. Add Chinese characters to `allChineseText` in `assets.cpp:241-405`
2. Characters are automatically extracted by `LoadCodepoints()` - no need to manually manage codepoints
3. Use `getText("English", "中文")` in UI code
4. Rebuild APK to test

### Adding New Game State
1. Add enum value to `GameState` in `game.h`
2. Add `update*()` and `draw*()` methods in `game.cpp`
3. Add state handling to `update()` and `draw()` switch statements
4. Add corresponding panel to `MenuPanel` enum in `ui.h`

### Adding New Settings Option
1. Add storage variable to `UIManager` class in `ui.h`
2. Add UI controls in `drawSettings()` (`ui.cpp:580-694`)
3. Add selection handler in `updateSettings()` (`game.cpp`)
4. Save/load logic if persistent storage needed
