#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <cmath>
#include <vector>

namespace BlockEater {

// Constants
constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TARGET_FPS = 60;

// World dimensions (4x screen size for larger map)
constexpr int WORLD_WIDTH = 5120;
constexpr int WORLD_HEIGHT = 2880;

// Game states
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    LEVEL_SELECT,
    SETTINGS,
    USER_MENU
};

// Game modes
enum class GameMode {
    ENDLESS,
    LEVEL,
    TIME_CHALLENGE
};

// Control modes
enum class ControlMode {
    VIRTUAL_JOYSTICK,
    TOUCH_FOLLOW
};

// Vector2 utility functions
inline Vector2 operator+(const Vector2& a, const Vector2& b) {
    return { a.x + b.x, a.y + b.y };
}

inline Vector2 operator-(const Vector2& a, const Vector2& b) {
    return { a.x - b.x, a.y - b.y };
}

inline Vector2 operator*(const Vector2& v, float s) {
    return { v.x * s, v.y * s };
}

inline float Vector2Length(const Vector2& v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

inline Vector2 Vector2Normalize(const Vector2& v) {
    float len = Vector2Length(v);
    if (len > 0.0001f) {
        return { v.x / len, v.y / len };
    }
    return v;
}

// Forward declarations
class Player;
class Enemy;
class ParticleSystem;
class UIManager;
class AudioManager;
class ControlSystem;
class AssetManager;
class GameCamera;
class Bullet;
class SkillManager;
class UserManager;

// Main Game class
class Game {
public:
    Game();
    ~Game();

    void init();
    void run();
    void update();
    void draw();
    void shutdown();

    // Getters
    GameState getState() const { return state; }
    GameMode getMode() const { return mode; }
    int getScore() const { return score; }
    float getDeltaTime() const { return deltaTime; }

    // Setters
    void setState(GameState s) { state = s; }
    void setMode(GameMode m) { mode = m; }
    void addScore(int s) { score += s; }

    // Game objects
    Player* player;
    std::vector<Enemy*> enemies;
    std::vector<Bullet*> bullets;
    ParticleSystem* particles;
    UIManager* ui;
    AudioManager* audio;
    ControlSystem* controls;
    AssetManager* assets;
    GameCamera* camera;
    SkillManager* skillManager;
    UserManager* userManager;

private:
    GameState state;
    GameState previousState;
    GameMode mode;
    ControlMode controlMode;
    int score;
    int currentLevel;
    float timeRemaining;
    float deltaTime;
    float gameTime;
    Texture2D backgroundTexture;  // Space background texture

    void updateMenu();
    void updatePlaying();
    void updatePaused();
    void updateGameOver();
    void updateLevelSelect();
    void updateSettings();
    void updateUserMenu();

    void drawMenu();
    void drawPlaying();
    void drawPaused();
    void drawGameOver();
    void drawLevelSelect();
    void drawUserMenu();

    void spawnEnemies();
    void checkCollisions();
    void drawBackground();
    void startGame(GameMode newMode);
    void resetGame();
};

} // namespace BlockEater

#endif // GAME_H
