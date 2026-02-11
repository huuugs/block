#include "ui.h"
#include "player.h"
#include "userManager.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdarg>

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

// Initialize static log buffer members
UIManager::LogEntry UIManager::logBuffer[MAX_LOG_ENTRIES];
int UIManager::logIndex = 0;
int UIManager::logCount = 0;

UIManager::UIManager()
    : menuAnimation(0)
    , hudAnimation(0)
    , transitionAlpha(0)
    , language(Language::ENGLISH)
    , currentThemeIndex(0)
    , currentTheme(&themes[0])
    , currentFontType(FontType::SOURCE_HAN_SANS)
    , currentControlMode(ControlMode::VIRTUAL_JOYSTICK)
    , masterVolume(0.8f)
    , m_isMuted(false)
    , currentPanel(MenuPanel::NONE)
    , previousPanel(MenuPanel::NONE)
    , mainMenuSelection(-1)
    , pauseMenuSelection(-1)
    , gameOverSelection(-1)
    , levelSelectSelection(-1)
    , settingsSelection(-1)
    , logsSelection(-1)
    , userMenuSelection(-1)
    , userSelection(-1)
    , selectedLevel(-1)
    , mainFont(nullptr)
    , secondaryFont(nullptr)
    , useCustomFont(false)
    , primaryColor{100, 200, 255, 255}      // Will be synced from theme in init()
    , secondaryColor{50, 100, 150, 255}    // Will be synced from theme in init()
    , accentColor{255, 200, 50, 255}       // Will be synced from theme in init()
    , backgroundColor{20, 20, 40, 220}     // Will be synced from theme in init()
{
    // Initialize local colors from theme
    primaryColor = currentTheme->primary;
    secondaryColor = currentTheme->secondary;
    accentColor = currentTheme->accent;
    backgroundColor = currentTheme->background;
}

UIManager::~UIManager() {
}

void UIManager::init(Font* mFont, Font* sFont) {
    mainFont = mFont;
    secondaryFont = sFont;
    useCustomFont = (mainFont != nullptr && mainFont->texture.id != 0);

    // Auto-detect system language on Android
    #if defined(PLATFORM_ANDROID)
    // Use JNI to get system locale
    // Default to Chinese if system locale is zh, otherwise English
    // This is a simple check - in production you'd want full locale support
    const char* locale = getenv("LANG");  // Some Android environments set this
    if (locale && (strstr(locale, "zh") || strstr(locale, "CN"))) {
        language = Language::CHINESE;
    }
    // Note: For full Android locale detection, you'd need JNI calls to Java
    #endif

    // CRITICAL: Set up raylib log callback to redirect all TraceLog to game log viewer
    SetTraceLogCallback([](int logLevel, const char* text, va_list args) -> void {
        // Redirect all raylib logs to game's log system
        switch (logLevel) {
            case LOG_INFO:
                UIManager::logInfo(text);
                break;
            case LOG_WARNING:
                UIManager::logWarning(text);
                break;
            case LOG_ERROR:
                UIManager::logError(text);
                break;
            case LOG_DEBUG:
            case LOG_NONE:
            default:
                UIManager::logInfo(text);
                break;
        }
    });

    logInfo("=== UI INIT START - Log capturing enabled ===");

    // Add initial logs for debugging
    logInfo("UIManager initialized");

    // Log detailed font information
    if (mainFont != nullptr) {
        char fontInfo[128];
        snprintf(fontInfo, sizeof(fontInfo), "Font ID: %u, size: %dx%d, baseSize: %d, glyphCount: %d",
            mainFont->texture.id, mainFont->texture.width, mainFont->texture.height, mainFont->baseSize, mainFont->glyphCount);
        logInfo(fontInfo);

        if (mainFont->texture.id == 1 || mainFont->texture.id == 0) {
            logWarning("Font texture ID is 0 or 1 (likely default font - NO CHINESE)");
        } else {
            logInfo("Custom font texture loaded");
        }

        // Check if font has enough glyphs for Chinese
        if (mainFont->glyphCount < 100) {
            char warnMsg[128];
            snprintf(warnMsg, sizeof(warnMsg), "WARNING: Only %d glyphs loaded (need ~200 for Chinese)", mainFont->glyphCount);
            logWarning(warnMsg);
        } else {
            char infoMsg[128];
            snprintf(infoMsg, sizeof(infoMsg), "Font has %d glyphs (should include Chinese)", mainFont->glyphCount);
            logInfo(infoMsg);
        }
    } else {
        logError("Font pointer is NULL!");
    }

    // Initialize raygui style
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    applyThemeToGui();

    // Set raygui font if custom font is available
    if (useCustomFont) {
        GuiSetFont(*mainFont);
        TraceLog(LOG_INFO, "UIManager: Using custom font for Chinese support");
    } else {
        TraceLog(LOG_INFO, "UIManager: Using default font (limited Chinese support)");
        logWarning("Using default font - Chinese may not display correctly");
    }

    // Log current language setting
    const char* langName = (language == Language::CHINESE) ? "Chinese" : "English";
    char langMsg[64];
    snprintf(langMsg, sizeof(langMsg), "Language set to: %s", langName);
    logInfo(langMsg);
}

void UIManager::applyThemeToGui() {
    // Apply current theme colors to raygui
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(currentTheme->background));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt(currentTheme->secondary));
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, ColorToInt(currentTheme->primary));
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(currentTheme->secondary));
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(currentTheme->accent));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(currentTheme->text));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(currentTheme->accent));
}

void UIManager::setLanguage(Language lang) {
    language = lang;
}

void UIManager::setCurrentPanel(MenuPanel panel) {
    if (currentPanel != panel) {
        previousPanel = currentPanel;
        currentPanel = panel;
        clearSelections();
        menuAnimation = 0.0f;  // Reset animation for new panel
    }
}

void UIManager::cycleTheme() {
    currentThemeIndex = (currentThemeIndex + 1) % NUM_THEMES;
    currentTheme = &themes[currentThemeIndex];

    // Sync local color variables with theme (for backward compatibility)
    primaryColor = currentTheme->primary;
    secondaryColor = currentTheme->secondary;
    accentColor = currentTheme->accent;
    backgroundColor = currentTheme->background;

    applyThemeToGui();
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

    // Draw log panel if active (has priority over normal state drawing)
    if (currentPanel == MenuPanel::LOGS) {
        drawLogs();
        return;
    }

    // Map GameState to MenuPanel for proper isolation
    MenuPanel targetPanel = MenuPanel::NONE;
    switch (state) {
        case GameState::MENU:
            targetPanel = MenuPanel::MAIN_MENU;
            break;
        case GameState::PLAYING:
            // HUD is drawn separately, no menu panel
            break;
        case GameState::PAUSED:
            targetPanel = MenuPanel::PAUSE_MENU;
            break;
        case GameState::GAME_OVER:
            targetPanel = MenuPanel::GAME_OVER;
            break;
        case GameState::LEVEL_SELECT:
            targetPanel = MenuPanel::LEVEL_SELECT;
            break;
        case GameState::SETTINGS:
            targetPanel = MenuPanel::SETTINGS;
            break;
    }

    // Update current panel if changed
    if (targetPanel != MenuPanel::NONE && currentPanel != targetPanel) {
        setCurrentPanel(targetPanel);
    }

    // Draw based on state
    switch (state) {
        case GameState::MENU:
            drawMainMenu();
            break;
        case GameState::PLAYING:
            // HUD is drawn by game.cpp with player reference
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

bool UIManager::drawButton(float x, float y, float width, float height, const char* text, bool enabled) {
    // Custom button drawing to support Chinese fonts properly
    // raygui doesn't reliably use custom fonts on Android

    Rectangle bounds = {x, y, width, height};

    // Check for hover/click state
    Vector2 mousePos = GetMousePosition();
    int touchCount = GetTouchPointCount();
    if (touchCount > 0) {
        mousePos = GetTouchPosition(0);
    }

    bool hovered = CheckCollisionPointRec(mousePos, bounds);
    bool clicked = false;

    if (enabled && hovered) {
        // Check for click (touch/mouse release)
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (touchCount > 0 && GetTouchPointCount() == 0)) {
            clicked = true;
        }
    }

    // Draw button background
    Color bgColor = enabled ? (hovered ? secondaryColor : primaryColor) : (Color){60, 60, 60, 200};
    DrawRectangle((int)x, (int)y, (int)width, (int)height, bgColor);
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, {150, 150, 200, 255});

    // Draw text with custom font
    int fontSize = 20;
    int textWidth = measureTextWithFont(text, fontSize);
    int textX = (int)(x + (width - textWidth) / 2);
    int textY = (int)(y + (height - fontSize) / 2);

    Color textColor = enabled ? WHITE : (Color){150, 150, 150, 255};
    drawTextWithFont(text, textX, textY, fontSize, textColor);

    return clicked;
}

void UIManager::drawMenuBackground() {
    // Semi-transparent overlay for menu panels
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 180});
}

void UIManager::drawMainMenu() {
    float alpha = menuAnimation;

    // Update menu animation
    if (menuAnimation < 1.0f) {
        menuAnimation += GetFrameTime() * 2.0f;
        if (menuAnimation > 1.0f) menuAnimation = 1.0f;
    }

    // Ensure raygui uses custom font for Chinese text rendering
    if (useCustomFont && mainFont != nullptr) {
        GuiSetFont(*mainFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    }

    // TEST: Draw Chinese characters directly at top of screen
    if (mainFont != nullptr && mainFont->glyphCount > 100) {
        // Test 1: ASCII (should work)
        DrawTextEx(*mainFont, "TEST ASCII", (Vector2){10, 10}, 24, 2, GREEN);

        // Test 2: Show first 10 Chinese glyphs that were actually loaded
        int displayY = 40;
        int chineseShown = 0;

        for (int i = 95; i < mainFont->glyphCount && chineseShown < 10; i++) {
            int charValue = mainFont->glyphs[i].value;
            if (charValue > 127) {
                // Draw the character using its Unicode value
                char str[5];
                int utf8Len = 0;
                // Convert Unicode to UTF-8
                if (charValue <= 0x7FF) {
                    str[0] = 0xC0 | (charValue >> 6);
                    str[1] = 0x80 | (charValue & 0x3F);
                    utf8Len = 2;
                } else if (charValue <= 0xFFFF) {
                    str[0] = 0xE0 | (charValue >> 12);
                    str[1] = 0x80 | ((charValue >> 6) & 0x3F);
                    str[2] = 0x80 | (charValue & 0x3F);
                    utf8Len = 3;
                }
                str[utf8Len] = '\0';

                DrawTextEx(*mainFont, str, (Vector2){10, (float)displayY}, 20, 1, YELLOW);
                displayY += 25;
                chineseShown++;
            }
        }
    }

    // Animated title
    const char* title = getText("BLOCK EATER", "方块吞噬者");
    int titleFontSize = 60;
    int titleWidth = measureTextWithFont(title, titleFontSize);

    float bounce = sinf(GetTime() * 3.0f) * 5.0f * alpha;
    Color titleColor = {255,
        static_cast<unsigned char>(200 + 55 * sinf(GetTime() * 3)),
        50, 255};
    drawTextWithFont(title, SCREEN_WIDTH / 2 - titleWidth / 2, 100 + (int)bounce, titleFontSize, titleColor);

    // Menu buttons using raygui with proper layout
    float buttonWidth = 280.0f;
    float buttonHeight = 50.0f;
    float startY = 220.0f;
    float spacing = 15.0f;
    float centerX = (float)(SCREEN_WIDTH / 2) - buttonWidth / 2;

    // Button 0: Play Endless
    if (drawButton(centerX, startY, buttonWidth, buttonHeight,
                   getText("PLAY ENDLESS", "无尽模式"))) {
        mainMenuSelection = 0;
    }

    // Button 1: Level Mode
    if (drawButton(centerX, startY + (buttonHeight + spacing) * 1, buttonWidth, buttonHeight,
                   getText("LEVEL MODE", "关卡模式"))) {
        mainMenuSelection = 1;
    }

    // Button 2: Time Challenge
    if (drawButton(centerX, startY + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight,
                   getText("TIME CHALLENGE", "时间挑战"))) {
        mainMenuSelection = 2;
    }

    // Button 3: Settings
    if (drawButton(centerX, startY + (buttonHeight + spacing) * 3, buttonWidth, buttonHeight,
                   getText("SETTINGS", "设置"))) {
        mainMenuSelection = 3;
    }

    // Button 4: Quit
    if (drawButton(centerX, startY + (buttonHeight + spacing) * 4, buttonWidth, buttonHeight,
                   getText("QUIT", "退出"))) {
        mainMenuSelection = 4;
    }

    // Instructions at bottom
    const char* instructions = getText("Touch left side to move", "触摸左半屏移动");
    int instrWidth = measureTextWithFont(instructions, 14);
    drawTextWithFont(instructions, SCREEN_WIDTH / 2 - instrWidth / 2, SCREEN_HEIGHT - 40, 14, {150, 150, 150, 255});

    // Version info
    drawTextWithFont("v1.0", 10, SCREEN_HEIGHT - 20, 12, {100, 100, 100, 200});
}

void UIManager::drawPauseMenu() {
    // Dark overlay
    drawMenuBackground();

    // Ensure raygui uses custom font for Chinese text rendering
    if (useCustomFont && mainFont != nullptr) {
        GuiSetFont(*mainFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    }

    // Pause text with pulse effect
    const char* text = getText("PAUSED", "暂停");
    int fontSize = 50;
    int textWidth = measureTextWithFont(text, fontSize);
    float pulse = 1.0f + sinf(GetTime() * 5.0f) * 0.05f;
    int scaledSize = (int)(fontSize * pulse);
    int scaledWidth = measureTextWithFont(text, scaledSize);
    drawTextWithFont(text, SCREEN_WIDTH / 2 - scaledWidth / 2, 150 - (scaledSize - fontSize) / 2, 
             scaledSize, WHITE);

    // Pause menu buttons
    float buttonWidth = 250.0f;
    float buttonHeight = 50.0f;
    float centerX = (float)(SCREEN_WIDTH / 2) - buttonWidth / 2;
    float startY = 280.0f;
    float spacing = 20.0f;

    // Button 0: Resume
    if (drawButton(centerX, startY, buttonWidth, buttonHeight,
                   getText("RESUME", "继续"))) {
        pauseMenuSelection = 0;
    }
    
    // Button 1: Settings
    if (drawButton(centerX, startY + buttonHeight + spacing, buttonWidth, buttonHeight,
                   getText("SETTINGS", "设置"))) {
        pauseMenuSelection = 1;
    }
    
    // Button 2: Quit to Menu
    if (drawButton(centerX, startY + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight,
                   getText("QUIT TO MENU", "退出到菜单"))) {
        pauseMenuSelection = 2;
    }
}

void UIManager::drawGameOverMenu(int score, int level) {
    // Red-tinted overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {50, 0, 0, 200});

    // Ensure raygui uses custom font for Chinese text rendering
    if (useCustomFont && mainFont != nullptr) {
        GuiSetFont(*mainFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    }

    // Game Over text with shake effect
    const char* text = getText("GAME OVER", "游戏结束");
    int fontSize = 60;
    float shake = sinf(GetTime() * 20.0f) * 2.0f;
    int textWidth = measureTextWithFont(text, fontSize);
    drawTextWithFont(text, SCREEN_WIDTH / 2 - textWidth / 2 + (int)shake, 100, 
             fontSize, {255, 50, 50, 255});

    // Score display
    char scoreText[64];
    if (language == Language::CHINESE) {
        sprintf(scoreText, "最终得分: %d", score);
    } else {
        sprintf(scoreText, "Final Score: %d", score);
    }
    int scoreFontSize = 30;
    int scoreWidth = measureTextWithFont(scoreText, scoreFontSize);
    drawTextWithFont(scoreText, SCREEN_WIDTH / 2 - scoreWidth / 2, 200, scoreFontSize, WHITE);

    // Level reached
    char levelText[64];
    if (language == Language::CHINESE) {
        sprintf(levelText, "达到等级: %d", level);
    } else {
        sprintf(levelText, "Level Reached: %d", level);
    }
    int levelWidth = measureTextWithFont(levelText, scoreFontSize);
    drawTextWithFont(levelText, SCREEN_WIDTH / 2 - levelWidth / 2, 250, scoreFontSize, {255, 200, 50, 255});

    // Game Over menu buttons
    float buttonWidth = 250.0f;
    float buttonHeight = 50.0f;
    float centerX = (float)(SCREEN_WIDTH / 2) - buttonWidth / 2;
    float startY = 350.0f;
    float spacing = 20.0f;

    // Button 0: Try Again
    if (drawButton(centerX, startY, buttonWidth, buttonHeight,
                   getText("TRY AGAIN", "再试一次"))) {
        gameOverSelection = 0;
    }
    
    // Button 1: Main Menu
    if (drawButton(centerX, startY + buttonHeight + spacing, buttonWidth, buttonHeight,
                   getText("MAIN MENU", "主菜单"))) {
        gameOverSelection = 1;
    }
}

void UIManager::drawLevelSelect() {
    // Ensure raygui uses custom font for Chinese text rendering
    if (useCustomFont && mainFont != nullptr) {
        GuiSetFont(*mainFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    }

    // Title
    const char* title = getText("SELECT LEVEL", "选择关卡");
    int fontSize = 40;
    int textWidth = measureTextWithFont(title, fontSize);
    drawTextWithFont(title, SCREEN_WIDTH / 2 - textWidth / 2, 50, fontSize, currentTheme->text);

    // Level buttons grid (10 levels in 2 rows)
    float buttonSize = 80.0f;
    float spacing = 20.0f;
    float totalWidth = 5 * buttonSize + 4 * spacing;
    float startX = ((float)SCREEN_WIDTH - totalWidth) / 2;
    float startY = 150.0f;

    for (int i = 0; i < 10; i++) {
        int row = i / 5;
        int col = i % 5;
        float x = startX + col * (buttonSize + spacing);
        float y = startY + row * (buttonSize + spacing);

        char levelText[16];
        sprintf(levelText, "%d", i + 1);

        if (drawButton(x, y, buttonSize, buttonSize, levelText)) {
            selectedLevel = i + 1;
            levelSelectSelection = 0;  // Confirm selection
        }
    }

    // Back button
    float backButtonWidth = 200.0f;
    float backButtonHeight = 50.0f;
    if (drawButton((float)(SCREEN_WIDTH / 2) - backButtonWidth / 2, 450.0f, 
                   backButtonWidth, backButtonHeight, getText("BACK", "返回"))) {
        levelSelectSelection = 1;  // Back
    }
}

void UIManager::drawSettings() {
    // Ensure raygui uses custom font for Chinese text rendering
    if (useCustomFont && mainFont != nullptr) {
        GuiSetFont(*mainFont);
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    }

    // Title
    const char* title = getText("SETTINGS", "设置");
    int fontSize = 40;
    int textWidth = measureTextWithFont(title, fontSize);
    drawTextWithFont(title, SCREEN_WIDTH / 2 - textWidth / 2, 50, fontSize, currentTheme->text);

    float startY = 140.0f;
    float spacing = 70.0f;
    float labelX = 200.0f;
    float valueX = 500.0f;
    float buttonWidth = 200.0f;
    float buttonHeight = 40.0f;

    // Language setting
    drawTextWithFont(getText("Language:", "语言:"), (int)labelX, (int)(startY + 15), 20, currentTheme->text);
    const char* langText = (language == Language::ENGLISH) ? "English" : "中文";
    if (drawButton(valueX, startY, buttonWidth, buttonHeight, langText)) {
        settingsSelection = 0;  // Toggle language
    }

    // Theme setting
    float themeY = startY + spacing;
    drawTextWithFont(getText("Theme:", "主题:"), (int)labelX, (int)(themeY + 15), 20, currentTheme->text);
    
    // Theme name button
    if (drawButton(valueX, themeY, buttonWidth, buttonHeight, currentTheme->name)) {
        settingsSelection = 1;  // Next theme
    }
    
    // Theme cycle button
    if (drawButton(valueX + buttonWidth + 20, themeY, 80.0f, buttonHeight, ">")) {
        cycleTheme();
    }

    // Control mode setting
    float controlY = startY + spacing * 2;
    drawTextWithFont(getText("Control:", "控制:"), (int)labelX, (int)(controlY + 15), 20, currentTheme->text);

    // Show current control mode
    const char* controlText = (currentControlMode == ControlMode::VIRTUAL_JOYSTICK)
        ? getText("Joystick", "摇杆")
        : getText("Touch Follow", "触摸跟随");
    if (drawButton(valueX, controlY, buttonWidth, buttonHeight, controlText)) {
        settingsSelection = 2;  // Toggle control mode
    }

    // Font setting
    float fontY = startY + spacing * 3;
    drawTextWithFont(getText("Font:", "字体:"), (int)labelX, (int)(fontY + 15), 20, currentTheme->text);

    // Show current font
    if (drawButton(valueX, fontY, buttonWidth, buttonHeight, getFontName())) {
        settingsSelection = 7;  // Cycle font
    }

    // Font cycle button
    if (drawButton(valueX + buttonWidth + 20, fontY, 80.0f, buttonHeight, ">")) {
        cycleFont();
    }

    // Volume sliders (interactive)
    float volumeY = startY + spacing * 4;
    drawTextWithFont(getText("Volume:", "音量:"), (int)labelX, (int)(volumeY + 15), 20, currentTheme->text);

    // Volume bar background
    DrawRectangle((int)valueX, (int)(volumeY + 10), 300, 20, {50, 50, 50, 200});
    // Volume level
    int volumeWidth = (int)(300 * masterVolume);
    DrawRectangle((int)valueX, (int)(volumeY + 10), volumeWidth, 20, m_isMuted ? (Color){80, 80, 80, 200} : (Color){50, 200, 50, 255});
    DrawRectangleLines((int)valueX, (int)(volumeY + 10), 300, 20, {150, 150, 150, 200});

    // Volume slider interaction (click to set volume)
    Vector2 sliderPos;
    int touchCount = GetTouchPointCount();
    if (touchCount > 0) {
        sliderPos = GetTouchPosition(0);
    } else {
        sliderPos = GetMousePosition();
    }

    Rectangle sliderBounds = {(float)valueX, volumeY, 300.0f, 20.0f};
    if (CheckCollisionPointRec(sliderPos, sliderBounds)) {
        if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && touchCount == 0) || touchCount > 0) {
            float relativeX = sliderPos.x - valueX;
            masterVolume = fmaxf(0.0f, fminf(1.0f, relativeX / 300.0f));
        }
    }

    // Mute button
    const char* muteText = m_isMuted ? getText("Unmute", "取消静音") : getText("Mute", "静音");
    float muteX = valueX + 320;
    if (drawButton(muteX, volumeY, 80.0f, 20, muteText)) {
        settingsSelection = 4;  // Toggle mute
    }

    // View Logs button
    float logsY = startY + spacing * 5;
    if (drawButton(valueX, logsY, buttonWidth, buttonHeight, getText("View Logs", "查看日志"))) {
        settingsSelection = 6;  // View logs
    }

    // Back button at bottom
    float backY = 620.0f;
    if (drawButton((float)(SCREEN_WIDTH / 2) - 100, backY, 200.0f, 50.0f,
                   getText("BACK", "返回"))) {
        settingsSelection = 5;  // Back
    }
}

void UIManager::drawHUD(const Player* player) {
    if (!player) return;

    // Top bar background with transparency
    drawPixelRect(0, 0, SCREEN_WIDTH, 60, backgroundColor);

    // Health bar with label
    drawHealthBar(20, 10, 200, 20, player->getHealth(), player->getMaxHealth(), {200, 50, 50, 255});
    const char* hpLabel = getText("HP", "生命");
    drawTextWithFont(hpLabel, 5, 12, 12, {255, 200, 200, 255});

    // Energy bar with label
    drawEnergyBar(20, 35, 200, 15, player->getEnergy(), player->getMaxEnergy());
    const char* energyLabel = getText("Energy", "能量");
    drawTextWithFont(energyLabel, 5, 37, 10, {200, 200, 255, 255});

    // Experience bar with label
    drawExpBar(240, 10, 300, 20, 0, 100, {50, 200, 100, 255});
}

void UIManager::drawHealthBar(float x, float y, float width, float height, int current, int max, Color color) {
    // Background
    DrawRectangle((int)x, (int)y, (int)width, (int)height, {50, 50, 50, 200});
    
    // Health fill
    float percentage = (float)current / (float)max;
    int fillWidth = (int)(width * percentage);
    DrawRectangle((int)x, (int)y, fillWidth, (int)height, color);
    
    // Border
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, {200, 200, 200, 100});
    
    // Text
    char text[32];
    sprintf(text, "%d/%d", current, max);
    int textWidth = measureTextWithFont(text, 12);
    drawTextWithFont(text, (int)(x + width / 2 - textWidth / 2), (int)(y + 4), 12, WHITE);
}

void UIManager::drawEnergyBar(float x, float y, float width, float height, float current, float max) {
    // Background
    DrawRectangle((int)x, (int)y, (int)width, (int)height, {50, 50, 50, 200});
    
    // Energy fill (blue)
    float percentage = current / max;
    int fillWidth = (int)(width * percentage);
    DrawRectangle((int)x, (int)y, fillWidth, (int)height, {50, 150, 255, 255});
    
    // Border
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, {200, 200, 200, 100});
}

void UIManager::drawExpBar(float x, float y, float width, float height, int current, int max, Color color) {
    // Background
    DrawRectangle((int)x, (int)y, (int)width, (int)height, {50, 50, 50, 200});
    
    // XP fill
    float percentage = (float)current / (float)max;
    int fillWidth = (int)(width * percentage);
    DrawRectangle((int)x, (int)y, fillWidth, (int)height, color);
    
    // Border
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, {200, 200, 200, 100});
    
    // Label
    const char* label = getText("XP", "经验");
    drawTextWithFont(label, (int)x - 30, (int)y + 2, 14, WHITE);
}

void UIManager::drawScore(int score) {
    char text[32];
    sprintf(text, "Score: %d", score);
    drawTextWithFont(text, 20, 70, 20, WHITE);
}

void UIManager::drawTimer(float time) {
    char text[32];
    int minutes = (int)(time / 60);
    int seconds = (int)(time) % 60;
    sprintf(text, "%02d:%02d", minutes, seconds);
    
    int textWidth = measureTextWithFont(text, 30);
    
    // Warning color for low time
    Color timeColor = (time < 30.0f) ? RED : WHITE;
    if (time < 30.0f && (int)(time * 2) % 2 == 0) {
        timeColor = {255, 100, 100, 255};  // Blink effect
    }
    
    drawTextWithFont(text, SCREEN_WIDTH / 2 - textWidth / 2, 20, 30, timeColor);
}

void UIManager::drawLevel(int level) {
    char text[32];
    sprintf(text, "Lv.%d", level);
    drawTextWithFont(text, SCREEN_WIDTH - 80, 20, 20, {255, 255, 100, 255});
}

void UIManager::drawMiniMap() {
    // Mini-map background
    int mapSize = 120;
    int mapX = SCREEN_WIDTH - mapSize - 20;
    int mapY = SCREEN_HEIGHT - mapSize - 20;
    
    DrawRectangle(mapX, mapY, mapSize, mapSize, {0, 0, 0, 150});
    DrawRectangleLines(mapX, mapY, mapSize, mapSize, {200, 200, 200, 100});
    
    // Player dot (center of minimap)
    int dotX = mapX + mapSize / 2;
    int dotY = mapY + mapSize / 2;
    DrawCircle(dotX, dotY, 4, GREEN);
}

const char* UIManager::getText(const char* english, const char* chinese) const {
    return (language == Language::CHINESE) ? chinese : english;
}

// Helper function to draw text with custom font support
void UIManager::drawTextWithFont(const char* text, int x, int y, int fontSize, Color color) {
    if (useCustomFont && mainFont != nullptr) {
        // Calculate font scale
        float scale = (float)fontSize / mainFont->baseSize;
        Vector2 position = {(float)x, (float)y};
        DrawTextEx(*mainFont, text, position, (float)fontSize, 1.0f, color);
    } else {
        // Fallback to default font
        DrawText(text, x, y, fontSize, color);
    }
}

// Helper function to measure text width with custom font
int UIManager::measureTextWithFont(const char* text, int fontSize) {
    if (useCustomFont && mainFont != nullptr) {
        Vector2 size = MeasureTextEx(*mainFont, text, (float)fontSize, 1.0f);
        return (int)size.x;
    } else {
        return MeasureText(text, fontSize);
    }
}

void UIManager::drawPixelButton(int x, int y, int width, int height, const char* text, bool hovered, bool pressed) {
    Color bgColor = hovered ? secondaryColor : primaryColor;
    if (pressed) bgColor = accentColor;
    
    // Draw button background
    DrawRectangle(x, y, width, height, bgColor);
    
    // Draw border
    DrawRectangleLines(x, y, width, height, {255, 255, 255, 200});
    
    // Draw text
    int textWidth = measureTextWithFont(text, 20);
    int textX = x + (width - textWidth) / 2;
    int textY = y + (height - 20) / 2;
    drawTextWithFont(text, textX, textY, 20, WHITE);
}

void UIManager::drawPixelRect(int x, int y, int width, int height, Color color, bool filled) {
    if (filled) {
        DrawRectangle(x, y, width, height, color);
    } else {
        DrawRectangleLines(x, y, width, height, color);
    }
}

void UIManager::drawPixelText(const char* text, int x, int y, int fontSize, Color color) {
    drawTextWithFont(text, x, y, fontSize, color);
}

// Log functions
void UIManager::logInfo(const char* message) {
    int idx = logIndex % MAX_LOG_ENTRIES;
    logCount = (logCount < MAX_LOG_ENTRIES) ? logCount + 1 : MAX_LOG_ENTRIES;
    logIndex++;
    snprintf(logBuffer[idx].message, MAX_LOG_LENGTH, "[INFO] %s", message);
    logBuffer[idx].type = 0;
}

void UIManager::logWarning(const char* message) {
    int idx = logIndex % MAX_LOG_ENTRIES;
    logCount = (logCount < MAX_LOG_ENTRIES) ? logCount + 1 : MAX_LOG_ENTRIES;
    logIndex++;
    snprintf(logBuffer[idx].message, MAX_LOG_LENGTH, "[WARN] %s", message);
    logBuffer[idx].type = 1;
}

void UIManager::logError(const char* message) {
    int idx = logIndex % MAX_LOG_ENTRIES;
    logCount = (logCount < MAX_LOG_ENTRIES) ? logCount + 1 : MAX_LOG_ENTRIES;
    logIndex++;
    snprintf(logBuffer[idx].message, MAX_LOG_LENGTH, "[ERROR] %s", message);
    logBuffer[idx].type = 2;
}

void UIManager::drawLogs() {
    // Dark background for logs
    drawMenuBackground();

    // Title
    const char* title = getText("SYSTEM LOGS", "系统日志");
    int fontSize = 40;
    int titleWidth = measureTextWithFont(title, fontSize);
    drawTextWithFont(title, SCREEN_WIDTH / 2 - titleWidth / 2, 50, fontSize, currentTheme->text);

    // Draw log entries
    float startY = 120;
    float lineHeight = 20;
    int displayCount = (logCount < MAX_LOG_ENTRIES) ? logCount : MAX_LOG_ENTRIES;

    // Scroll to show newest logs first
    int startIdx = (logIndex - displayCount + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
    if (startIdx < 0) startIdx += MAX_LOG_ENTRIES;

    for (int i = 0; i < displayCount; i++) {
        int idx = (startIdx + i) % MAX_LOG_ENTRIES;
        float y = startY + i * lineHeight;

        // Color based on log type
        Color logColor = WHITE;
        if (logBuffer[idx].type == 1) logColor = {255, 200, 100, 255};  // Warning - yellow
        if (logBuffer[idx].type == 2) logColor = {255, 100, 100, 255};  // Error - red

        drawTextWithFont(logBuffer[idx].message, 20, (int)y, 14, logColor);
    }

    // Draw log count info
    char countText[64];
    const char* logCountMsg = getText("Showing %d/%d logs", "显示 %d/%d 条日志");
    snprintf(countText, sizeof(countText), logCountMsg, displayCount, displayCount);
    drawTextWithFont(countText, 20, SCREEN_HEIGHT - 80, 14, {150, 150, 150, 255});

    // Back button
    float backButtonWidth = 200.0f;
    float backButtonHeight = 50.0f;
    if (drawButton((float)(SCREEN_WIDTH / 2) - 100, SCREEN_HEIGHT - 60,
                   backButtonWidth, backButtonHeight,
                   getText("BACK", "返回"))) {
        logsSelection = 0;  // Back
    }
}

// Font control methods
void UIManager::setFontType(FontType type) {
    currentFontType = type;
}

void UIManager::cycleFont() {
    switch (currentFontType) {
        case FontType::SOURCE_HAN_SANS:
            currentFontType = FontType::ZPIX;
            break;
        case FontType::ZPIX:
            currentFontType = FontType::DEFAULT;
            break;
        case FontType::DEFAULT:
            currentFontType = FontType::ZPIX;  // Skip SOURCE_HAN_SANS (removed)
            break;
    }
}

const char* UIManager::getFontName() const {
    switch (currentFontType) {
        case FontType::SOURCE_HAN_SANS:
            return getText("Zpix", "像素字体");  // Remapped to Zpix
        case FontType::ZPIX:
            return getText("Zpix", "像素字体");
        case FontType::DEFAULT:
            return getText("Default", "默认字体");
        default:
            return getText("Unknown", "未知");
    }
}

void UIManager::reloadFonts(Font* mainFont, Font* sFont) {
    if (mainFont && mainFont->texture.id != 0) {
        this->mainFont = mainFont;
        this->secondaryFont = sFont;
        this->useCustomFont = true;
        logInfo("Fonts reloaded successfully");
    }
}

void UIManager::drawUserMenu(const UserManager* userManager) {
    if (!userManager) {
        logError("drawUserMenu: userManager is null");
        return;
    }

    float centerX = SCREEN_WIDTH / 2.0f;
    float centerY = SCREEN_HEIGHT / 2.0f;
    
    // Draw title
    const char* title = getText("USER SYSTEM", "用户系统");
    int titleWidth = MeasureText(title, 40);
    DrawText(title, (int)(centerX - titleWidth / 2), 80, 40, currentTheme->text);
    
    // Draw current user
    const User* currentUser = userManager->getCurrentUser();
    if (currentUser) {
        char currentUserText[128];
        snprintf(currentUserText, sizeof(currentUserText),
                 getText("Current User: %s", "当前用户: %s"), currentUser->username);
        int userTextWidth = MeasureText(currentUserText, 24);
        DrawText(currentUserText, (int)(centerX - userTextWidth / 2), 140, 24, currentTheme->accent);
    }
    
    // Draw user list
    float startY = 200;
    float buttonHeight = 60;
    float buttonSpacing = 10;
    
    for (int i = 0; i < UserManager::MAX_USERS; i++) {
        const User* user = userManager->getUser(i);
        if (user && user->isValid) {
            float y = startY + i * (buttonHeight + buttonSpacing);
            
            // Draw user button background
            Rectangle buttonRect = {centerX - 200, y, 400, buttonHeight};
            Color buttonColor = currentTheme->secondary;
            
            // Highlight current user
            if (currentUser == user) {
                buttonColor = currentTheme->primary;
            }
            
            DrawRectangleRec(buttonRect, buttonColor);
            DrawRectangleLinesEx(buttonRect, 2, currentTheme->accent);
            
            // Draw username
            DrawText(user->username, (int)(centerX - 180), (int)(y + 20), 20, currentTheme->text);
            
            // Draw stats
            char statsText[64];
            snprintf(statsText, sizeof(statsText), "High Score: %d", user->totalScore);
            DrawText(statsText, (int)(centerX + 50), (int)(y + 20), 16, currentTheme->text);
            
            // Check for click
            int touchCount = GetTouchPointCount();
            Vector2 pos = touchCount > 0 ? GetTouchPosition(0) : GetMousePosition();
            
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || touchCount == 0) {
                if (CheckCollisionPointRec(pos, buttonRect)) {
                    userSelection = i;
                }
            }
        }
    }
    
    // Draw Create New User button
    float createY = startY + UserManager::MAX_USERS * (buttonHeight + buttonSpacing) + 20;
    if (drawButton(centerX - 100, createY, 200, 60,
                    getText("Create User", "创建用户"))) {
        userMenuSelection = 1;  // Create
    }
    
    // Draw Back button
    float backY = createY + 80;
    if (drawButton(centerX - 100, backY, 200, 60,
                    getText("Back", "返回"))) {
        userMenuSelection = 2;  // Back
    }
}

} // namespace BlockEater
