#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "game.h"
#include "particles.h"
#include "raygui.h"

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

// Button states
enum class ButtonState {
    IDLE,
    HOVER,
    PRESSED
};

// UI Button structure for raygui
struct UIButton {
    Rectangle bounds;
    const char* text;
    bool pressed;
    bool hovered;
    
    UIButton(float x, float y, float width, float height, const char* t)
        : bounds{x, y, width, height}, text(t), pressed(false), hovered(false) {}
    
    void update(Vector2 mousePos, bool mousePressed) {
        hovered = CheckCollisionPointRec(mousePos, bounds);
        pressed = hovered && mousePressed;
    }
    
    bool isClicked(Vector2 mousePos, bool mouseReleased) {
        return hovered && mouseReleased;
    }
};

// Menu panel for proper isolation
enum class MenuPanel {
    NONE,
    MAIN_MENU,
    PAUSE_MENU,
    GAME_OVER,
    LEVEL_SELECT,
    SETTINGS
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    void init(Font* mainFont, Font* sFont);
    void update(float dt);
    void draw(GameState state, GameMode mode);
    void resetTransition() { transitionAlpha = 0.0f; }
    void resetAnimation() { menuAnimation = 0.0f; hudAnimation = 0.0f; }

    // Language and theme
    void setLanguage(Language lang);
    Language getLanguage() const { return language; }
    void cycleTheme();
    Theme* getCurrentTheme() { return currentTheme; }

    // Control mode
    void setControlMode(ControlMode mode) { currentControlMode = mode; }
    ControlMode getControlMode() const { return currentControlMode; }

    // Text translation helper
    const char* getText(const char* english, const char* chinese);

    // Menu handling with proper isolation
    MenuPanel getCurrentPanel() const { return currentPanel; }
    void setCurrentPanel(MenuPanel panel);

    // Button interaction helpers
    bool drawButton(float x, float y, float width, float height, const char* text, bool enabled = true);
    bool drawIconButton(float x, float y, float size, const char* icon, bool enabled = true);
    
    // HUD elements
    void drawHUD(const Player* player);
    void drawHealthBar(float x, float y, float width, float height, int current, int max, Color color);
    void drawEnergyBar(float x, float y, float width, float height, float current, float max);
    void drawExpBar(float x, float y, float width, float height, int current, int max, Color color);
    void drawMiniMap();
    void drawScore(int score);
    void drawTimer(float time);
    void drawLevel(int level);

    // Menu panels (isolated)
    void drawMainMenu();
    void drawPauseMenu();
    void drawGameOverMenu(int score, int level);
    void drawLevelSelect();
    void drawSettings();

    // Get button click results
    int getMainMenuSelection() const { return mainMenuSelection; }
    int getPauseMenuSelection() const { return pauseMenuSelection; }
    int getGameOverSelection() const { return gameOverSelection; }
    int getLevelSelectSelection() const { return levelSelectSelection; }
    int getSettingsSelection() const { return settingsSelection; }
    int getSelectedLevel() const { return selectedLevel; }
    
    void clearSelections() {
        mainMenuSelection = -1;
        pauseMenuSelection = -1;
        gameOverSelection = -1;
        levelSelectSelection = -1;
        settingsSelection = -1;
        selectedLevel = -1;
    }

private:
    // Animations
    float menuAnimation;
    float hudAnimation;
    float transitionAlpha;

    // Language and theme
    Language language;
    Theme* currentTheme;
    int currentThemeIndex;

    // Control mode (for settings display)
    ControlMode currentControlMode;

    // Current UI panel for isolation
    MenuPanel currentPanel;
    MenuPanel previousPanel;

    // Button selections (set when buttons are clicked)
    int mainMenuSelection;
    int pauseMenuSelection;
    int gameOverSelection;
    int levelSelectSelection;
    int settingsSelection;
    int selectedLevel;

    // Colors (deprecated, using themes now)
    Color primaryColor;
    Color secondaryColor;
    Color accentColor;
    Color backgroundColor;

    // Static themes array
    static const int NUM_THEMES = 5;
    static Theme themes[];

    // Font references
    Font* mainFont;
    Font* secondaryFont;
    bool useCustomFont;

    // Helper functions
    void drawPixelButton(int x, int y, int width, int height, const char* text, bool hovered, bool pressed);
    void drawPixelRect(int x, int y, int width, int height, Color color, bool filled = true);
    void drawPixelText(const char* text, int x, int y, int fontSize, Color color);
    void applyThemeToGui();
    void drawMenuBackground();
    void drawPanel(MenuPanel panel, float alpha);
    
    // Font-aware text drawing
    void drawTextWithFont(const char* text, int x, int y, int fontSize, Color color);
    int measureTextWithFont(const char* text, int fontSize);
};

} // namespace BlockEater

#endif // UI_H
