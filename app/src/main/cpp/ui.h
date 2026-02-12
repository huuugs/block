#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "game.h"
#include "particles.h"
#include "raygui.h"

namespace BlockEater {

// Forward declaration
class UserManager;

// Language support
enum class Language {
    ENGLISH,
    CHINESE
};

// Font type for font switching
enum class FontType {
    SOURCE_HAN_SANS,    // Full CJK support
    ZPIX,               // Pixel style, limited Chinese
    DEFAULT             // Raylib default (no Chinese)
};

// Theme colors
struct Theme {
    Color primary;
    Color secondary;
    Color accent;
    Color background;
    Color text;
    const char* name;
    const char* nameCN;  // Chinese name
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
    SETTINGS,
    LOGS,
    USER_MENU
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

    // Volume control
    void setMasterVolume(float volume) { masterVolume = volume; }
    float getMasterVolume() const { return masterVolume; }
    bool isMuted() const { return m_isMuted; }
    void toggleMute() { m_isMuted = !m_isMuted; }

    // Font control
    void setFontType(FontType type);
    FontType getFontType() const { return currentFontType; }
    void cycleFont();
    const char* getFontName() const;
    void reloadFonts(Font* mainFont, Font* sFont);

    // Text translation helper
    const char* getText(const char* english, const char* chinese) const;

    // User management
    void setUserManager(const UserManager* um) { userManager = um; }

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
    void drawLevelSelect(const UserManager* userManager);
    void drawSettings();
    void drawLogs();
    void drawUserMenu(const UserManager* userManager);
    void drawNameInput(const char* nameBuffer);  // Draw name input screen
    void drawUserStats(const UserManager* userManager, int userIndex);  // Draw detailed user stats
    void drawDeleteConfirm(const UserManager* userManager, int userIndex);  // Draw delete confirmation

    // Logging system for debugging
    static void logInfo(const char* message);
    static void logWarning(const char* message);
    static void logError(const char* message);

    // Get button click results
    int getMainMenuSelection() const { return mainMenuSelection; }
    int getPauseMenuSelection() const { return pauseMenuSelection; }
    int getGameOverSelection() const { return gameOverSelection; }
    int getLevelSelectSelection() const { return levelSelectSelection; }
    int getSettingsSelection() const { return settingsSelection; }
    int getLogsSelection() const { return logsSelection; }
    int getUserMenuSelection() const { return userMenuSelection; }
    int getUserSelection() const { return userSelection; }
    int getDeleteUserConfirm() const { return deleteUserConfirm; }
    int getUserToDelete() const { return userToDelete; }
    int getSelectedLevel() const { return selectedLevel; }

    void setDeleteConfirm(int confirm) { deleteUserConfirm = confirm; }
    void setUserToDelete(int userIndex) { userToDelete = userIndex; }

    void clearSelections() {
        mainMenuSelection = -1;
        pauseMenuSelection = -1;
        gameOverSelection = -1;
        levelSelectSelection = -1;
        settingsSelection = -1;
        logsSelection = -1;
        userMenuSelection = -1;
        userSelection = -1;
        selectedLevel = -1;
        deleteUserConfirm = -1;
        userToDelete = -1;
    }

private:
    // Log buffer
    static const int MAX_LOG_ENTRIES = 50;
    static const int MAX_LOG_LENGTH = 256;
    struct LogEntry {
        char message[MAX_LOG_LENGTH];
        int type;  // 0=info, 1=warning, 2=error
    };
    static LogEntry logBuffer[MAX_LOG_ENTRIES];
    static int logIndex;
    static int logCount;

    // Animations
    float menuAnimation;
    float hudAnimation;
    float transitionAlpha;

    // Language and theme
    Language language;
    Theme* currentTheme;
    int currentThemeIndex;

    // Font control
    FontType currentFontType;

    // Control mode (for settings display)
    ControlMode currentControlMode;

    // Volume control
    float masterVolume;
    bool m_isMuted;

    // Current UI panel for isolation
    MenuPanel currentPanel;
    MenuPanel previousPanel;

    // Button selections (set when buttons are clicked)
    int mainMenuSelection;
    int pauseMenuSelection;
    int gameOverSelection;
    int levelSelectSelection;
    int settingsSelection;
    int logsSelection;
    int userMenuSelection;
    int userSelection;
    int selectedLevel;
    int deleteUserConfirm;  // For delete user confirmation
    int userToDelete;     // Which user to delete

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

    // User manager reference
    const UserManager* userManager;

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
