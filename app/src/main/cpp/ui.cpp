#include "ui.h"
#include "player.h"
#include "raygui.h"
#include <cstdio>
#include <cmath>

namespace BlockEater {

// Define static theme colors array
Theme UIManager::themes[UIManager::NUM_THEMES] = {
    // Default Blue Theme
    {{100, 200, 255, 255}, {50, 100, 150, 255}, {255, 200, 50, 255}, {20, 20, 40, 255}, {255, 255, 255, 255}, "Blue"},
    // Dark Theme
    {{80, 80, 100, 255}, {40, 40, 60, 255}, {150, 150, 180, 255}, {15, 15, 25, 255}, {200, 200, 220, 255}, "Dark"},
    // Green Theme
    {{100, 220, 120, 255}, {50, 150, 80, 255}, {255, 220, 100, 255}, {20, 35, 25, 255}, {255, 255, 255, 255}, "Green"},
    // Purple Theme
    {{180, 120, 255, 255}, {120, 60, 180, 255}, {255, 180, 100, 255}, {30, 20, 45, 255}, {255, 255, 255, 255}, "Purple"},
    // Red Theme
    {{255, 120, 100, 255}, {180, 60, 50, 255}, {255, 220, 50, 255}, {40, 20, 20, 255}, {255, 255, 255, 255}, "Red"}
};

UIManager::UIManager()
    : menuAnimation(0)
    , hudAnimation(0)
    , transitionAlpha(0)
    , language(Language::ENGLISH)
    , currentThemeIndex(0)
    , currentTheme(&themes[0])
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

    // Update menu animation
    if (menuAnimation < 1.0f) {
        menuAnimation += GetFrameTime() * 2.0f;
        if (menuAnimation > 1.0f) menuAnimation = 1.0f;
    }

    // Title
    const char* title = getText("BLOCK EATER", "方块吞噬者");
    int titleFontSize = 60;
    int titleWidth = MeasureText(title, titleFontSize);

    Color titleColor = {255, static_cast<unsigned char>(200 + 55 * sinf(GetTime() * 3)), 50, 255};
    DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2, 100, titleFontSize, titleColor);

    // Menu buttons using raygui
    float buttonWidth = 280;
    float buttonHeight = 50;
    float startY = 220;
    float spacing = 10;

    // Draw buttons (visual only, click handling in game.cpp)
    GuiButton((Rectangle){SCREEN_WIDTH/2 - buttonWidth/2, startY, buttonWidth, buttonHeight},
              getText("PLAY ENDLESS", "无尽模式"));
    GuiButton((Rectangle){SCREEN_WIDTH/2 - buttonWidth/2, startY + (buttonHeight + spacing) * 1, buttonWidth, buttonHeight},
              getText("LEVEL MODE", "关卡模式"));
    GuiButton((Rectangle){SCREEN_WIDTH/2 - buttonWidth/2, startY + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight},
              getText("TIME CHALLENGE", "时间挑战"));
    GuiButton((Rectangle){SCREEN_WIDTH/2 - buttonWidth/2, startY + (buttonHeight + spacing) * 3, buttonWidth, buttonHeight},
              getText("SETTINGS", "设置"));
    GuiButton((Rectangle){SCREEN_WIDTH/2 - buttonWidth/2, startY + (buttonHeight + spacing) * 4, buttonWidth, buttonHeight},
              getText("QUIT", "退出"));

    // Instructions
    const char* instructions = getText("Touch left side to move", "触摸左半屏移动");
    int instrWidth = MeasureText(instructions, 14);
    DrawText(instructions, SCREEN_WIDTH / 2 - instrWidth / 2, SCREEN_HEIGHT - 40, 14, {150, 150, 150, 255});
}

void UIManager::drawPauseMenu() {
    // Overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 180});

    // Pause text
    const char* text = getText("PAUSED", "暂停");
    int fontSize = 50;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH / 2 - textWidth / 2, 150, fontSize, WHITE);

    // Buttons using raygui
    int buttonWidth = 250;
    int buttonHeight = 50;

    GuiButton((Rectangle){SCREEN_WIDTH / 2 - buttonWidth / 2, 250, (float)buttonWidth, (float)buttonHeight},
              getText("RESUME", "继续"));
    GuiButton((Rectangle){SCREEN_WIDTH / 2 - buttonWidth / 2, 320, (float)buttonWidth, (float)buttonHeight},
              getText("QUIT TO MENU", "退出到菜单"));
}

void UIManager::drawGameOverMenu(int score, int level) {
    // Overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {50, 0, 0, 200});

    // Game Over text
    const char* text = getText("GAME OVER", "游戏结束");
    int fontSize = 60;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH / 2 - textWidth / 2, 100, fontSize, {255, 50, 50, 255});

    // Score
    char scoreText[64];
    if (language == Language::CHINESE) {
        sprintf(scoreText, "最终得分: %d", score);
    } else {
        sprintf(scoreText, "Final Score: %d", score);
    }
    int scoreFontSize = 30;
    int scoreWidth = MeasureText(scoreText, scoreFontSize);
    DrawText(scoreText, SCREEN_WIDTH / 2 - scoreWidth / 2, 200, scoreFontSize, WHITE);

    // Level reached
    char levelText[64];
    if (language == Language::CHINESE) {
        sprintf(levelText, "达到等级: %d", level);
    } else {
        sprintf(levelText, "Level Reached: %d", level);
    }
    int levelWidth = MeasureText(levelText, scoreFontSize);
    DrawText(levelText, SCREEN_WIDTH / 2 - levelWidth / 2, 250, scoreFontSize, {255, 200, 50, 255});

    // Buttons using raygui
    int buttonWidth = 250;
    int buttonHeight = 50;

    GuiButton((Rectangle){SCREEN_WIDTH / 2 - buttonWidth / 2, 350, (float)buttonWidth, (float)buttonHeight},
              getText("TRY AGAIN", "再试一次"));
    GuiButton((Rectangle){SCREEN_WIDTH / 2 - buttonWidth / 2, 420, (float)buttonWidth, (float)buttonHeight},
              getText("MAIN MENU", "主菜单"));
}

void UIManager::drawLevelSelect() {
    const char* title = getText("SELECT LEVEL", "选择关卡");
    int fontSize = 40;
    int textWidth = MeasureText(title, fontSize);
    DrawText(title, SCREEN_WIDTH / 2 - textWidth / 2, 50, fontSize, currentTheme->text);

    // Level buttons (10 levels in 2 rows) using raygui
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

        GuiButton((Rectangle){(float)x, (float)y, (float)buttonSize, (float)buttonSize}, levelText);
    }

    // Back button
    GuiButton((Rectangle){SCREEN_WIDTH / 2 - 100, 500, 200, 50}, getText("BACK", "返回"));
}

void UIManager::drawSettings() {
    const char* title = getText("SETTINGS", "设置");
    int fontSize = 40;
    int textWidth = MeasureText(title, fontSize);
    DrawText(title, SCREEN_WIDTH / 2 - textWidth / 2, 50, fontSize, currentTheme->text);

    // Language setting
    DrawText(getText("Language:", "语言:"), 200, 135, 20, currentTheme->text);
    const char* langText = (language == Language::ENGLISH) ? "English" : "中文";
    GuiButton((Rectangle){500, 120, 200, 40}, langText);

    // Theme setting
    DrawText(getText("Theme:", "主题:"), 200, 205, 20, currentTheme->text);
    GuiButton((Rectangle){500, 190, 200, 40}, currentTheme->name);
    GuiButton((Rectangle){720, 190, 80, 40}, getText(">", ">"));

    // Control mode setting
    DrawText(getText("Control Mode:", "控制模式:"), 200, 275, 20, currentTheme->text);
    GuiButton((Rectangle){500, 260, 200, 40}, getText("Virtual Joystick", "虚拟摇杆"));
    GuiButton((Rectangle){720, 260, 200, 40}, getText("Touch Follow", "触摸跟随"));

    // Volume settings (visual only, using DrawRectangle)
    DrawText(getText("Master Volume:", "主音量:"), 200, 355, 20, currentTheme->text);
    DrawRectangle(500, 350, 300, 20, {100, 100, 100, 255});
    DrawRectangle(500, 350, 200, 20, {50, 200, 50, 255});

    DrawText(getText("SFX Volume:", "音效音量:"), 200, 405, 20, currentTheme->text);
    DrawRectangle(500, 400, 300, 20, {100, 100, 100, 255});
    DrawRectangle(500, 400, 250, 20, {50, 150, 255, 255});

    DrawText(getText("Music Volume:", "音乐音量:"), 200, 455, 20, currentTheme->text);
    DrawRectangle(500, 450, 300, 20, {100, 100, 100, 255});
    DrawRectangle(500, 450, 150, 20, {255, 150, 50, 255});

    // Back button
    GuiButton((Rectangle){SCREEN_WIDTH / 2 - 100, 530, 200, 50}, getText("BACK", "返回"));
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

const char* UIManager::getText(const char* english, const char* chinese) {
    return (language == Language::CHINESE) ? chinese : english;
}

void UIManager::cycleTheme() {
    currentThemeIndex = (currentThemeIndex + 1) % NUM_THEMES;
    currentTheme = const_cast<Theme*>(&themes[currentThemeIndex]);
    // Update old color variables for compatibility
    primaryColor = currentTheme->primary;
    secondaryColor = currentTheme->secondary;
    accentColor = currentTheme->accent;
    backgroundColor = currentTheme->background;
}

} // namespace BlockEater
