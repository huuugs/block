#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "game.h"
#include "particles.h"

namespace BlockEater {

// Language support
enum class Language {
    ENGLISH,
    CHINESE
};

// Theme colors
struct Theme {
    Color primary;
    Color secondary;
    Color accent;
    Color background;
    Color text;
    const char* name;
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    void update(float dt);
    void draw(GameState state, GameMode mode);
    void resetTransition() { transitionAlpha = 0.0f; }

    // Language and theme
    void setLanguage(Language lang) { language = lang; }
    Language getLanguage() const { return language; }
    void cycleTheme();
    Theme* getCurrentTheme() { return currentTheme; }

    // Text translation helper
    const char* getText(const char* english, const char* chinese);

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
    float transitionAlpha;  // For smooth state transitions

    // Language and theme
    Language language;
    Theme* currentTheme;
    int currentThemeIndex;

    // Button helpers
    void drawPixelButton(int x, int y, int width, int height, const char* text, bool hovered, bool pressed);
    void drawPixelRect(int x, int y, int width, int height, Color color, bool filled = true);
    void drawPixelText(const char* text, int x, int y, int fontSize, Color color);

    // Colors (deprecated, using themes now)
    Color primaryColor;
    Color secondaryColor;
    Color accentColor;
    Color backgroundColor;

    // Static themes array
    static const int NUM_THEMES = 5;
    static Theme themes[];
};

} // namespace BlockEater

#endif // UI_H
