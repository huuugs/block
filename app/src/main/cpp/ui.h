#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "game.h"
#include "particles.h"

namespace BlockEater {

class UIManager {
public:
    UIManager();
    ~UIManager();

    void update(float dt);
    void draw(GameState state, GameMode mode);

    // HUD elements
    void drawHUD(const Player* player);
    void drawHealthBar(float x, float y, float width, float height, int current, int max, Color color);
    void drawEnergyBar(float x, float y, float width, float height, float current, float max);
    void drawExpBar(float x, float y, float width, float height, int current, int max, Color color);
    void drawMiniMap();
    void drawScore(int score);
    void drawTimer(float time);
    void drawLevel(int level);

    // Menu elements
    void drawMainMenu();
    void drawPauseMenu();
    void drawGameOverMenu(int score, int level);
    void drawLevelSelect();
    void drawSettings();

    // Button handling
    bool isButtonClicked(int x, int y, int width, int height);
    void drawButton(int x, int y, int width, int height, const char* text, bool hovered);

private:
    // Animations
    float menuAnimation;
    float hudAnimation;

    // Button helpers
    void drawPixelButton(int x, int y, int width, int height, const char* text, bool hovered, bool pressed);
    void drawPixelRect(int x, int y, int width, int height, Color color, bool filled = true);
    void drawPixelText(const char* text, int x, int y, int fontSize, Color color);

    // Colors
    Color primaryColor;
    Color secondaryColor;
    Color accentColor;
    Color backgroundColor;
};

} // namespace BlockEater

#endif // UI_H
