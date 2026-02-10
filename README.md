# Block Eater - Raylib Android Game

A retro pixel-style block eating game built with raylib for Android.

## Game Features

- **Three Game Modes**: Endless, Level-based (10 levels), and Time Challenge
- **Player Progression**: 6 levels with increasing size, health, armor
- **Four Enemy Types**: Floating, Chasing, Stationary, Bouncing
- **Dual Control Modes**: Virtual Joystick and Touch Follow
- **Retro Pixel Art Style**: 8-bit inspired visuals and UI
- **Procedural Audio**: 8-bit chiptune sound effects generated at runtime

## Project Structure

```
/root/block/
├── app/
│   └── src/main/
│       ├── java/com/blockeater/MainActivity.java
│       ├── cpp/                          # Game C++ source
│       │   ├── main.cpp                  # Entry point
│       │   ├── game.h/cpp                # Main game class
│       │   ├── player.h/cpp              # Player class
│       │   ├── enemy.h/cpp               # Enemy class
│       │   ├── particles.h/cpp           # Particle system
│       │   ├── ui.h/cpp                  # UI system
│       │   ├── audio.h/cpp               # Audio system
│       │   ├── modes.h/cpp               # Game modes
│       │   ├── controls.h/cpp            # Controls
│       │   ├── assets.h/cpp              # Asset generation
│       │   └── CMakeLists.txt            # CMake config
│       ├── AndroidManifest.xml
│       └── build.gradle.kts
├── raylib/                               # Raylib source (downloaded)
├── build.gradle.kts
├── settings.gradle.kts
└── build-game.sh                         # Build script
```

## Build Requirements

- Android NDK 25.x
- CMake 3.20+
- Gradle 8.0+
- Android SDK API 24+ (Android 7.0+)

## Building

### Quick Start

```bash
# Full build (downloads raylib, builds everything)
./build-game.sh all

# Or step by step:
./build-game.sh setup    # Download and build raylib
./build-game.sh build    # Build APK
./build-game.sh install  # Install to connected device
```

### Build Commands

| Command | Description |
|---------|-------------|
| `./build-game.sh setup` | Download and build raylib for Android |
| `./build-game.sh raylib` | Only build raylib |
| `./build-game.sh build` | Build APK |
| `./build-game.sh install` | Install APK to device |
| `./build-game.sh quick` | Quick build (skip raylib download) |
| `./build-game.sh clean` | Clean build files |

## Player Levels

| Level | Size | Health | Armor | Color | Speed |
|-------|------|--------|-------|-------|-------|
| 1 | 30px | 100 | 0 | Green | 200 |
| 2 | 40px | 200 | 5 | Cyan | 190 |
| 3 | 50px | 350 | 10 | Blue | 180 |
| 4 | 65px | 550 | 15 | Purple | 170 |
| 5 | 80px | 800 | 20 | Pink | 160 |
| 6 | 100px | 1100 | 30 | Gold | 150 |

## Controls

### Virtual Joystick Mode
- Touch and drag on the left side of the screen
- 360-degree directional control
- Configurable joystick position

### Touch Follow Mode
- Touch and hold anywhere on screen
- Block moves toward touch point
- Supports multi-touch

## Game Modes

### Endless Mode
- Continuous gameplay
- Difficulty increases over time
- Target: Highest score and level

### Level Mode
- 10 predefined levels
- Each level has specific objectives
- Reach score, level, or survive time limit

### Time Challenge
- 3-minute time limit
- Eating blocks adds bonus time
- Score as high as possible

## License

This project uses raylib, licensed under Zlib/libpng license.

## Credits

- **raylib** by Ray Tracer (raysan5)
- Built with C++17 and CMake
- Targeting Android 7.0+ (API 24+)
