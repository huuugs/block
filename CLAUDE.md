# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Block Eater is a raylib-based Android game written in C++17. It features a block-eating gameplay mechanic with multiple game modes, procedural audio, touch controls, multi-language support (English/Chinese), a user system with persistent statistics, and physics-based movement.

**Current Development Focus:** All major features are implemented. The game is feature-complete.

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

**Required Environment:**
- JDK 17
- Android NDK 25.2.9519653
- Gradle 8.5
- CMake 3.22+

### APK Signing Configuration

The project uses a **fixed signing keystore** stored in GitHub Secrets to ensure consistent APK signatures for upgrade installation.

**Signing Config:**
- Store password: `blockeater2024`
- Key alias: `blockeaterdebug`
- Config name: `blockeaterDebug`
- Keystore path: `~/.android/debug.keystore`

**Local Development:**
- For local builds, the signing config expects:
  - Keystore path: `~/.android/debug.keystore`
- Store password: `blockeater2024`

## Architecture

### Game State Machine

The `Game` class (`game.h/cpp`) manages the entire game through a state machine:

- **GameState::MENU** - Main menu with mode selection
- **GameState::PLAYING** - Active gameplay
- **GameState::PAUSED** - Pause menu
- **GameState::GAME_OVER** - Game over screen
- **GameState::LEVEL_SELECT** - Level selection
- **GameState::SETTINGS** - Settings (language, theme, controls, volume)
- **GameState::USER_MENU** - User system (create, select, delete, view stats)
- **GameState::NAME_INPUT** - Username input for creating new users
- **GameState::NAME_INPUT** - Username input for creating new users

### Code Structure

All game code lives in `app/src/main/cpp/`:

**Core Files:**
- `game.h/cpp` - Main game class, state machine, entity management
- `player.h/cpp` - Player entity with 15 progression levels
- `enemy.h/cpp` - Four enemy types with physics
- `bullet.h/cpp` - Projectile system
- `ui.h/cpp` - Menu system, multi-language support, themes
- `audio.h/cpp` - Procedural 8-bit sound generation
- `modes.h/cpp` - Game mode logic (Endless, Level, Time Challenge)
- `controls.cpp` - Virtual joystick and touch-follow input
- `particles.h/cpp` - Visual effects
- **assets.h/cpp** - Runtime asset generation and font loading
- `userManager.h/cpp` - User data persistence
- `skills.h/cpp` - 4 skills with cooldowns
- `camera.h/cpp` - Camera following player

**Namespace:**
All code uses `BlockEater::` namespace.

## Game Controls

### Virtual Joystick (Default)
- Touch left half of screen to spawn joystick at touch position
- Joystick only draws when active (finger touching)
- `controls.cpp:114-130` - Joystick activation logic

### Touch Follow Mode
- Move toward touch point relative to player position

### Keyboard Controls (Desktop Testing)
- Arrow keys (UP, DOWN, LEFT, RIGHT)
- WASD keys
- Handled in `controls.cpp:47-58`

### Quick Save
- **Keys:** F5 or ESC
- **Cooldown:** 2 seconds between saves
- **Implementation:** `game.cpp:120-130`
- **Save Location:** Updates current user statistics automatically

## Game Modes

### Endless Mode
- Continuous gameplay with increasing difficulty
- `difficultyMultiplier` increases over time (0.01 per second)
- Spawn rate formula: `fmaxf(0.3f, 2.0f - level * 0.15f)`
- No win condition

### Level Mode
- 10 predefined levels with unlock system
- **Unlock System:** Complete current level objectives (score + level) to unlock next level
- **First level** unlocked by default, others locked
- `modes.cpp:59-70` - Level definitions with `unlocked` field
- **Difficulty Scaling:**
  - Enemy count: `5 + level * 3`
  - Spawn rate: `1.8f - level * 0.1f`
- **Victory Conditions:**
  - Target score: `500 * level`
  - Target level: `level` + 1`
  - Time limits vary by level

### Time Challenge
- 3-minute countdown
- Eating food blocks adds +5 seconds per block

## User System

### User Data Structure (`user.h/cpp`)
```cpp
struct User {
    char username[64];           // Display name
    int totalGamesPlayed;       // Total games across all modes
    float totalPlayTime;         // Total play time in seconds
    int totalScore;             // Aggregate score across all games
    int maxLevelUnlocked;      // Highest level beaten in Level Mode
    ModeStats endlessStats;       // Endless mode statistics
    ModeStats levelStats;         // Level mode statistics
    ModeStats timeChallengeStats;  // Time Challenge statistics
};
```

### Supported Operations
- **Create User** - `createUser(username)` - Max 5 users
- **Delete User** - `deleteUser(index)` - Cannot delete current user
- **Select User** - `setCurrentUser(index)` - Switch active user
- **Save Data** - Automatically saved on game events and quick save
- **Export/Import** - `exportToFile(path)` / `importFromFile(path)`

### Statistics Tracking
All gameplay updates call `userManager->updateStats(mode, score, playTime, levelReached)`:
- Tracks: high scores, games played, play time per mode

## UI System

### Language Support
- **Default Language:** Chinese (for Android users)
- **Toggle Method:** Settings option or via key binding
- **Implementation:** `ui.cpp:70-80` - `setLanguage(Language::CHINESE)`

### Theme System
Five built-in themes:
- Blue, Dark, Green, Purple, Red
- **Control:** `cycleTheme()` in Settings

### Font System
- **Current:** Zpix (pixel font, ~7MB, ~3500 Chinese characters)
- **Loading:** `LoadFileData()` → `LoadFontFromMemory()` method

## Audio System

### Procedural 8-bit Sound Generation
- Implemented in `audio.cpp:1-250`
- **Instruments:** Square, Sawtooth, Triangle
- **Effects:** Volume, attack, eat, death, level up, button click

## Physics System

### Implementation
- **Library:** Custom physics using raylib Vector2
- **Movement:** Force-based with acceleration (`applyForce()`)
- **Collision:** Elastic response with momentum conservation
- **Formula:** `v1' = v1 - 2*m2/(m1+m2) * dot(v1-v2,n)`

## Known Issues & Solutions

### Fixed: Shield Not Displaying
- **Problem:** Skill 4 (shield) didn't appear when used
- **Solution:** Added `shieldTimeLeft = shieldDuration;` initialization (`skills.cpp:161`)

### Fixed: Chinese Font Support
- **Problem:** `LoadFontEx()` couldn't access assets directory on Android
- **Solution:** Use `LoadFileData()` + `LoadFontFromMemory()` pattern

### Fixed: Skills Cannot Respond Simultaneously
- **Problem:** Using joystick prevented skill activation
- **Solution:** Iterate through ALL touch points when detecting skill clicks (`game.cpp:290-347`)

### Fixed: Touch Point ID Tracking
- **Problem:** Touch point indices (0,1,2...) change when fingers are released
- **Solution:** Use `GetTouchPointId()` to track actual touch point ID (`controls.cpp:103`)

### Fixed: APK Signing Inconsistency
- **Problem:** Each CI build used different keystore, preventing upgrade installation
- **Solution:** Fixed keystore stored in GitHub Secrets (`DEBUG_KEYSTORE_BASE64`)

### Fixed: Vector2 Unary Minus
- **Problem:** Using `-normal` caused compilation error
- **Solution:** Manual negation `(Vector2){-normal.x, -normal.y}`

## Debugging

### Log Viewer
- **Access:** Settings > "View Logs" button
- **Auto-Capture:** All raylib logs redirected to in-game log viewer
- **Implementation:** `ui.cpp:890-935`

## Development Commands

### Adding New Content
When adding new content:
1. Update relevant sections in CLAUDE.md
2. Add new enum values, constants, or game states
3. Document file locations (e.g., font files, level definitions)
4. Update build commands if new files are added

### Coding Conventions

1. **C++17 Standard:**
   - Use `nullptr` for null pointers
   - Use `override` for method overrides
   - Use `= default` for member initialization
   - Use `const` for constants
   - Follow raylib naming conventions (PascalCase for functions)

2. **State Management:**
   - Always call `ui->resetTransition()` when switching states
   - Use pattern: `state = GameState::NEW_STATE` then add handling

3. **Physics:**
   - Force-based movement requires mass calculation
   - Remember: `mass = DENSITY * size * size * size`

4. **Input Handling:**
   - Check ALL touch points for UI interactions
   - Use `GetTouchPointCount()` then iterate with `GetTouchPosition(i)`
   - Keyboard: Use `IsKeyPressed()` and `GetCharPressed()` for desktop testing

5. **Multi-touch Support:**
   - First touch (0): Joystick control
   - Second touch (1): Skills activation
   - Critical for simultaneous interactions

## Contact

For questions about this codebase:
- Review recent commit history
- Check existing issues in GitHub Issues
- Consult CLAUDE.md for architecture overview
- Test on multiple Android devices for touch input variance

## File Manifest

### AndroidManifest.xml
```xml
<manifest>
    <meta-data android:name="android.app.lib_name" android:value="main" />
</manifest>
```

### MainActivity.java
- NativeActivity auto-loads the raylib library via manifest metadata
- No manual `System.loadLibrary()` call needed

## Progress Tracking

### Completed Features
- [x] Core gameplay (block eating, physics, enemy AI)
- [x] Multi-game modes (Endless, Level, Time Challenge)
- [x] Touch controls (virtual joystick, touch-follow)
- [x] Audio system (procedural sounds)
- [x] User system (5 slots, statistics)
- [x] UI system (multi-language, themes, custom fonts)
- [x] Level system (10 levels, progressive difficulty)
- [x] Skill system (4 skills with cooldowns)
- [x] Particle effects
- [x] Quick save (F5/Esc, 2s cooldown)
- [x] Physics-based movement
- [x] Rigid body collision response
- [x] Android build configuration
- [x] APK signing with fixed keystore
- [x] Chinese font support (3500+ characters)

### TODO / Future Enhancements
- None - Game is feature-complete

## Key Technical Decisions

1. **Static Library Linking:** Raylib linked as static library with `--whole-archive`
2. **32-bit Only:** Disabled due to NEON compilation issues in raylib 5.0
3. **C++17:** Modern C++ features with std::vector usage
4. **OpenGL ES 3.0:** Hardware accelerated rendering
5. **File I/O:** Standard C library methods for save/load

## Quick Reference

### Creating New Game State

1. Add enum value to `GameState` in `game.h`:
   ```cpp
   enum class GameState { ..., NEW_STATE };
   ```

2. Add update handler in `game.cpp: update()`:
   ```cpp
   case GameState::NEW_STATE:
       updateNewState();
       break;
   ```

3. Add draw handler in `game.cpp: draw()`:
   ```cpp
   case GameState::NEW_STATE:
       drawNewState();
       break;
   ```

4. Implement handler functions:
   ```cpp
   void Game::updateNewState() { /* ... */ }
   void Game::drawNewState() { /* ... */ }
   ```

5. Update UI manager in `ui.h/cpp`:
   - Add panel enum value to `MenuPanel`
   - Add `drawNewPanel()` method
   - Update `draw()` to handle new panel

6. Test thoroughly with different game modes and states
