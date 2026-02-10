#include "ui.h"
#include "player.h"
#include <cstdio>
#include <cmath>

namespace BlockEater {

UIManager::UIManager()
    : menuAnimation(0)
    , hudAnimation(0)
    , transitionAlpha(0)
    , primaryColor{100, 200, 255, 255}
    , secondaryColor{50, 100, 150, 255}
    , accentColor{255, 200, 50, 255}
    , backgroundColor{20, 20, 40, 220}
{
}

UIManager::~UIManager() {
}

void UIManager::update(float dt) {
    if (menuAnimation < 1.0f) {
        menuAnimation += dt * 2.0f;
        if (menuAnimation > 1.0f) menuAnimation = 1.0f;
    }

    if (hudAnimation < 1.0f) {
        hudAnimation += dt * 3.0f;
        if (hudAnimation > 1.0f) hudAnimation = 1.0f;
    }
}

void UIManager::draw(GameState state, GameMode mode) {
    // Fade in effect when transitioning to a new state
    if (transitionAlpha < 1.0f) {
        transitionAlpha += GetFrameTime() * 3.0f;
        if (transitionAlpha > 1.0f) transitionAlpha = 1.0f;
    }

    switch (state) {
        case GameState::MENU:
            drawMainMenu();
            break;
        case GameState::PLAYING:
            drawHUD(nullptr);  // Will be called with player from game
            break;
        case GameState::PAUSED:
            drawPauseMenu();
            break;
        case GameState::GAME_OVER:
            drawGameOverMenu(0, 1);
            break;
        case GameState::LEVEL_SELECT:
            drawLevelSelect();
            break;
        case GameState::SETTINGS:
            drawSettings();
            break;
    }

    // Draw fade overlay if transitioning
    if (transitionAlpha < 1.0f) {
        unsigned char alpha = static_cast<unsigned char>((1.0f - transitionAlpha) * 255);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, alpha});
    }
}

void UIManager::drawHUD(const Player* player) {
    if (!player) return;

    // Top bar background
    drawPixelRect(0, 0, SCREEN_WIDTH, 60, backgroundColor);

    // Health bar
    drawHealthBar(20, 10, 200, 20, player->getHealth(), player->getMaxHealth(), {200, 50, 50, 255});

    // Energy bar
    drawEnergyBar(20, 35, 200, 15, player->getEnergy(), player->getMaxEnergy());

    // Experience bar
    drawExpBar(240, 10, 300, 20, 0, 100, {50, 200, 100, 255});

    // Level indicator
    drawLevel(player->getLevel());

    // Score (placeholder)
    drawScore(0);
}

void UIManager::drawHealthBar(float x, float y, float width, float height, int current, int max, Color color) {
    // Background
    drawPixelRect((int)x, (int)y, (int)width, (int)height, {30, 30, 30, 255});

    // Fill
    float fillWidth = (float)current / max * width;
    drawPixelRect((int)x, (int)y, (int)fillWidth, (int)height, color);

    // Border
    drawPixelRect((int)x, (int)y, (int)width, (int)height, WHITE, false);

    // Text
    char text[32];
    sprintf(text, "HP: %d/%d", current, max);
    DrawText(text, (int)(x + 5), (int)(y + 2), 10, WHITE);
}

void UIManager::drawEnergyBar(float x, float y, float width, float height, float current, float max) {
    // Background
    drawPixelRect((int)x, (int)y, (int)width, (int)height, {30, 30, 30, 255});

    // Fill
    float fillWidth = current / max * width;
    drawPixelRect((int)x, (int)y, (int)fillWidth, (int)height, {50, 150, 255, 255});

    // Border
    drawPixelRect((int)x, (int)y, (int)width, (int)height, WHITE, false);
}

void UIManager::drawExpBar(float x, float y, float width, float height, int current, int max, Color color) {
    // Background
    drawPixelRect((int)x, (int)y, (int)width, (int)height, {30, 30, 30, 255});

    // Fill (placeholder - using current XP ratio)
    float fillRatio = 0.5f;  // Will be updated by player
    float fillWidth = fillRatio * width;
    drawPixelRect((int)x, (int)y, (int)fillWidth, (int)height, color);

    // Border
    drawPixelRect((int)x, (int)y, (int)width, (int)height, WHITE, false);

    // Text
    DrawText("EXP", (int)(x + 5), (int)(y + 2), 10, WHITE);
}

void UIManager::drawMiniMap() {
    // Mini map in corner
    int mapSize = 100;
    int x = SCREEN_WIDTH - mapSize - 20;
    int y = SCREEN_HEIGHT - mapSize - 100;

    drawPixelRect(x, y, mapSize, mapSize, {0, 0, 0, 150});
    drawPixelRect(x, y, mapSize, mapSize, {100, 100, 150, 255}, false);
}

void UIManager::drawScore(int score) {
    char text[32];
    sprintf(text, "SCORE: %d", score);

    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH - textWidth - 20, 10, fontSize, WHITE);
}

void UIManager::drawTimer(float time) {
    int minutes = (int)(time / 60);
    int seconds = (int)(time) % 60;

    char text[32];
    sprintf(text, "%02d:%02d", minutes, seconds);

    int fontSize = 30;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH / 2 - textWidth / 2, 70, fontSize, WHITE);
}

void UIManager::drawLevel(int level) {
    char text[32];
    sprintf(text, "Level %d", level);
    DrawText(text, 550, 35, 14, {255, 255, 100, 255});
}

void UIManager::drawMainMenu() {
    float alpha = menuAnimation;

    // Title
    const char* title = "BLOCK EATER";
    int titleFontSize = 60;
    int titleWidth = MeasureText(title, titleFontSize);

    Color titleColor = {255, static_cast<unsigned char>(200 + 55 * sinf(GetTime() * 3)), 50, 255};
    DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2, 100, titleFontSize, titleColor);

    // Menu buttons
    int startY = 250;
    int buttonHeight = 60;
    int buttonWidth = 300;
    int gap = 80;

    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, startY, buttonWidth, buttonHeight,
                   "PLAY ENDLESS", false, false);
    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, startY + gap, buttonWidth, buttonHeight,
                   "LEVEL MODE", false, false);
    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, startY + gap * 2, buttonWidth, buttonHeight,
                   "TIME CHALLENGE", false, false);
    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, startY + gap * 3, buttonWidth, buttonHeight,
                   "SETTINGS", false, false);
    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, startY + gap * 4, buttonWidth, buttonHeight,
                   "QUIT", false, false);

    // Instructions
    const char* instructions = "Touch controls enabled";
    int instrWidth = MeasureText(instructions, 16);
    DrawText(instructions, SCREEN_WIDTH / 2 - instrWidth / 2, SCREEN_HEIGHT - 50, 16, {150, 150, 150, 255});
}

void UIManager::drawPauseMenu() {
    // Overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 180});

    // Pause text
    const char* text = "PAUSED";
    int fontSize = 50;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH / 2 - textWidth / 2, 150, fontSize, WHITE);

    // Buttons
    int buttonWidth = 250;
    int buttonHeight = 50;

    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, 250, buttonWidth, buttonHeight,
                   "RESUME", false, false);
    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, 320, buttonWidth, buttonHeight,
                   "QUIT", false, false);
}

void UIManager::drawGameOverMenu(int score, int level) {
    // Overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {50, 0, 0, 200});

    // Game Over text
    const char* text = "GAME OVER";
    int fontSize = 60;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH / 2 - textWidth / 2, 100, fontSize, {255, 50, 50, 255});

    // Score
    char scoreText[64];
    sprintf(scoreText, "Final Score: %d", score);
    int scoreFontSize = 30;
    int scoreWidth = MeasureText(scoreText, scoreFontSize);
    DrawText(scoreText, SCREEN_WIDTH / 2 - scoreWidth / 2, 200, scoreFontSize, WHITE);

    // Level reached
    char levelText[64];
    sprintf(levelText, "Level Reached: %d", level);
    int levelWidth = MeasureText(levelText, scoreFontSize);
    DrawText(levelText, SCREEN_WIDTH / 2 - levelWidth / 2, 250, scoreFontSize, {255, 200, 50, 255});

    // Buttons
    int buttonWidth = 250;
    int buttonHeight = 50;

    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, 350, buttonWidth, buttonHeight,
                   "TRY AGAIN", false, false);
    drawPixelButton(SCREEN_WIDTH / 2 - buttonWidth / 2, 420, buttonWidth, buttonHeight,
                   "MAIN MENU", false, false);
}

void UIManager::drawLevelSelect() {
    const char* title = "SELECT LEVEL";
    int fontSize = 40;
    int textWidth = MeasureText(title, fontSize);
    DrawText(title, SCREEN_WIDTH / 2 - textWidth / 2, 50, fontSize, WHITE);

    // Level buttons (10 levels in 2 rows)
    int buttonSize = 80;
    int startX = (SCREEN_WIDTH - 5 * (buttonSize + 20)) / 2 + 10;
    int startY = 150;

    for (int i = 0; i < 10; i++) {
        int row = i / 5;
        int col = i % 5;
        int x = startX + col * (buttonSize + 20);
        int y = startY + row * (buttonSize + 20);

        char levelText[16];
        sprintf(levelText, "%d", i + 1);

        drawPixelButton(x, y, buttonSize, buttonSize, levelText, false, false);
    }
}

void UIManager::drawSettings() {
    const char* title = "SETTINGS";
    int fontSize = 40;
    int textWidth = MeasureText(title, fontSize);
    DrawText(title, SCREEN_WIDTH / 2 - textWidth / 2, 50, fontSize, WHITE);

    // Control mode setting
    DrawText("Control Mode:", 200, 150, 24, WHITE);
    drawPixelButton(500, 140, 200, 50, "Virtual Joystick", false, false);
    drawPixelButton(720, 140, 200, 50, "Touch Follow", false, false);

    // Volume settings
    DrawText("Master Volume:", 200, 220, 24, WHITE);
    drawPixelRect(500, 230, 300, 20, {100, 100, 100, 255});
    drawPixelRect(500, 230, 200, 20, {50, 200, 50, 255});

    DrawText("SFX Volume:", 200, 280, 24, WHITE);
    drawPixelRect(500, 290, 300, 20, {100, 100, 100, 255});
    drawPixelRect(500, 290, 250, 20, {50, 150, 255, 255});

    DrawText("Music Volume:", 200, 340, 24, WHITE);
    drawPixelRect(500, 350, 300, 20, {100, 100, 100, 255});
    drawPixelRect(500, 350, 150, 20, {255, 150, 50, 255});

    // Back button
    drawPixelButton(SCREEN_WIDTH / 2 - 100, 500, 200, 50, "BACK", false, false);
}

bool UIManager::isButtonClicked(int x, int y, int width, int height) {
    Vector2 mouse = GetMousePosition();
    Vector2 touch = {0, 0};

    if (GetTouchPointCount() > 0) {
        touch = GetTouchPosition(0);
    }

    return (mouse.x >= x && mouse.x <= x + width && mouse.y >= y && mouse.y <= y + height) ||
           (touch.x >= x && touch.x <= x + width && touch.y >= y && touch.y <= y + height);
}

void UIManager::drawButton(int x, int y, int width, int height, const char* text, bool hovered) {
    drawPixelButton(x, y, width, height, text, hovered, false);
}

void UIManager::drawPixelButton(int x, int y, int width, int height, const char* text, bool hovered, bool pressed) {
    Color bgColor = hovered ? Color{80, 120, 180, 255} : Color{60, 80, 120, 255};
    if (pressed) bgColor = Color{40, 60, 100, 255};

    drawPixelRect(x, y, width, height, bgColor);

    // Pixel border
    drawPixelRect(x, y, width, height, {150, 180, 220, 255}, false);

    // Highlight
    drawPixelRect(x + 2, y + 2, width - 4, 3, {200, 220, 255, 150});

    // Text
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, x + (width - textWidth) / 2, y + (height - fontSize) / 2, fontSize, WHITE);
}

void UIManager::drawPixelRect(int x, int y, int width, int height, Color color, bool filled) {
    if (filled) {
        DrawRectangle(x, y, width, height, color);
    } else {
        DrawRectangleLines(x, y, width, height, color);
    }
}

void UIManager::drawPixelText(const char* text, int x, int y, int fontSize, Color color) {
    DrawText(text, x, y, fontSize, color);
}

} // namespace BlockEater
