#include "game.h"
#include "player.h"
#include "enemy.h"
#include "particles.h"
#include "ui.h"
#include "audio.h"
#include "modes.h"
#include "controls.h"
#include "assets.h"
#include "camera.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

using namespace BlockEater;

Game::Game()
    : player(nullptr)
    , particles(nullptr)
    , ui(nullptr)
    , audio(nullptr)
    , controls(nullptr)
    , assets(nullptr)
    , camera(nullptr)
    , state(GameState::MENU)
    , previousState(GameState::MENU)
    , mode(GameMode::ENDLESS)
    , controlMode(ControlMode::VIRTUAL_JOYSTICK)
    , score(0)
    , currentLevel(1)
    , timeRemaining(0)
    , deltaTime(0)
    , gameTime(0)
{
}

Game::~Game() {
}

void Game::init() {
    // Initialize managers
    assets = new AssetManager();
    assets->init();

    audio = new AudioManager();
    audio->init();

    controls = new ControlSystem();
    controls->init();

    ui = new UIManager();
    // Initialize UI with fonts (if loaded)
    ui->init(&assets->GetPixelFont(), &assets->GetSmallFont());

    particles = new ParticleSystem();

    // Initialize camera
    camera = new GameCamera();
    camera->init();

    // Create player
    player = new Player();
}

void Game::run() {
    while (!WindowShouldClose()) {
        deltaTime = GetFrameTime();

        update();
        draw();
    }
}

void Game::update() {
    controls->update();

    switch (state) {
        case GameState::MENU:
            updateMenu();
            break;
        case GameState::PLAYING:
            updatePlaying();
            break;
        case GameState::PAUSED:
            updatePaused();
            break;
        case GameState::GAME_OVER:
            updateGameOver();
            break;
        case GameState::LEVEL_SELECT:
            updateLevelSelect();
            break;
        case GameState::SETTINGS:
            updateSettings();
            break;
    }

    ui->update(deltaTime);
    particles->update(deltaTime);
}

void Game::draw() {
    BeginDrawing();
    ClearBackground({20, 20, 40, 255});

    drawBackground();

    switch (state) {
        case GameState::MENU:
            drawMenu();
            break;
        case GameState::PLAYING:
            drawPlaying();
            break;
        case GameState::PAUSED:
            drawPaused();
            break;
        case GameState::GAME_OVER:
            drawGameOver();
            break;
        case GameState::LEVEL_SELECT:
            drawLevelSelect();
            break;
        case GameState::SETTINGS:
            drawPaused();
            break;
    }

    particles->draw();

    EndDrawing();
}

void Game::shutdown() {
    // Clear enemies
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    // Delete managers
    delete player;
    delete particles;
    delete ui;
    delete audio;
    delete controls;
    delete assets;
    delete camera;
}

void Game::updateMenu() {
    // Check for menu button clicks via raygui
    int selection = ui->getMainMenuSelection();
    
    if (selection >= 0) {
        audio->playButtonClickSound();
        
        switch (selection) {
            case 0:  // Play Endless
                startGame(GameMode::ENDLESS);
                break;
            case 1:  // Level Mode
                state = GameState::LEVEL_SELECT;
                ui->resetTransition();
                break;
            case 2:  // Time Challenge
                startGame(GameMode::TIME_CHALLENGE);
                break;
            case 3:  // Settings
                state = GameState::SETTINGS;
                ui->resetTransition();
                break;
            case 4:  // Quit
                // Exit the game loop
                CloseWindow();
                break;
        }
        
        ui->clearSelections();
    }
}

void Game::updatePlaying() {
    // Update camera first (follow player)
    camera->update(player->getPosition(), deltaTime);

    // Update player
    Vector2 input = controls->getInputVector();
    player->move(input);
    player->update(deltaTime);

    // Update enemies
    for (auto* enemy : enemies) {
        enemy->update(deltaTime, player->getPosition());
    }

    // Check collisions
    checkCollisions();

    // Spawn enemies
    spawnEnemies();

    // Check game over
    if (player->getHealth() <= 0) {
        state = GameState::GAME_OVER;
        audio->playDeathSound();
    }

    // Update game time
    gameTime += deltaTime;

    // Update UI
    ui->update(deltaTime);

    // Check for menu button click (top right corner)
    if ((GetTouchPointCount() > 0) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = (GetTouchPointCount() > 0) ? GetTouchPosition(0) : GetMousePosition();
        // Menu button area: x=SCREEN_WIDTH-80 to SCREEN_WIDTH-10, y=10 to 45
        if (pos.x >= SCREEN_WIDTH - 80 && pos.x <= SCREEN_WIDTH - 10 &&
            pos.y >= 10 && pos.y <= 45) {
            state = GameState::PAUSED;
            audio->playButtonClickSound();
        }
    }
}

void Game::updatePaused() {
    // Handle Pause Menu
    if (state == GameState::PAUSED) {
        int selection = ui->getPauseMenuSelection();
        
        if (selection >= 0) {
            audio->playButtonClickSound();
            
            switch (selection) {
                case 0:  // Resume
                    state = GameState::PLAYING;
                    break;
                case 1:  // Settings (from pause)
                    previousState = state;
                    state = GameState::SETTINGS;
                    ui->resetTransition();
                    break;
                case 2:  // Quit to menu
                    state = GameState::MENU;
                    resetGame();
                    ui->resetTransition();
                    break;
            }
            
            ui->clearSelections();
        }
    }
}

    // Store current state before switching to settings
    if (state != GameState::SETTINGS) {
        previousState = state;
    }

void Game::updateSettings() {
    int selection = ui->getSettingsSelection();
    
    if (selection >= 0) {
        audio->playButtonClickSound();
        
        switch (selection) {
            case 0:  // Toggle Language
                ui->setLanguage(ui->getLanguage() == Language::ENGLISH ? Language::CHINESE : Language::ENGLISH);
                break;
            case 1:  // Next Theme
                ui->cycleTheme();
                break;
            case 2:  // Toggle Control Mode
                controlMode = (controlMode == ControlMode::VIRTUAL_JOYSTICK) 
                    ? ControlMode::TOUCH_FOLLOW 
                    : ControlMode::VIRTUAL_JOYSTICK;
                break;
            case 3:  // Back
                state = previousState;
                ui->resetTransition();
                break;
        }
        
        ui->clearSelections();
    }
}

void Game::updateGameOver() {
    // Check for try again/main menu buttons via raygui
    int selection = ui->getGameOverSelection();
    
    if (selection >= 0) {
        audio->playButtonClickSound();
        
        switch (selection) {
            case 0:  // Try Again
                startGame(mode);
                break;
            case 1:  // Main Menu
                state = GameState::MENU;
                resetGame();
                ui->resetTransition();
                break;
        }
        
        ui->clearSelections();
    }
}

void Game::updateLevelSelect() {
    // Check for level selection via raygui
    int selection = ui->getLevelSelectSelection();
    int selectedLevel = ui->getSelectedLevel();
    
    if (selection >= 0 || selectedLevel >= 0) {
        audio->playButtonClickSound();
        
        if (selectedLevel >= 0) {
            // Level selected
            currentLevel = selectedLevel;
            startGame(GameMode::LEVEL);
        } else if (selection == 1) {
            // Back button
            state = GameState::MENU;
            ui->resetTransition();
        }
        
        ui->clearSelections();
    }
}

void Game::drawBackground() {
    // Draw grid background
    int gridSize = 40;
    Color gridColor = {30, 30, 50, 255};
    Color bgColor = {15, 15, 30, 255};

    for (int x = 0; x < SCREEN_WIDTH; x += gridSize) {
        DrawLine(x, 0, x, SCREEN_HEIGHT, gridColor);
    }
    for (int y = 0; y < SCREEN_HEIGHT; y += gridSize) {
        DrawLine(0, y, SCREEN_WIDTH, y, gridColor);
    }
}

void Game::drawMenu() {
    ui->draw(state, mode);
    controls->draw();
}

void Game::drawPlaying() {
    // Apply camera for world rendering
    camera->apply();

    // Draw player
    player->draw();

    // Draw enemies
    for (auto* enemy : enemies) {
        enemy->draw();
    }

    // End camera mode (switch back to screen space for UI)
    camera->end();

    // Draw UI (in screen space)
    ui->drawHUD(player);
    ui->drawScore(score);

    if (mode == GameMode::TIME_CHALLENGE || mode == GameMode::LEVEL) {
        ui->drawTimer(timeRemaining);
    }

    // Draw menu button (top right corner)
    const char* menuText = ui->getText("MENU", "菜单");
    int menuFontSize = 16;
    int menuTextWidth = MeasureText(menuText, menuFontSize);
    DrawRectangle(SCREEN_WIDTH - 80, 10, 70, 35, {50, 50, 80, 200});
    DrawRectangleLines(SCREEN_WIDTH - 80, 10, 70, 35, {100, 100, 150, 255});
    DrawText(menuText, SCREEN_WIDTH - 80 + (70 - menuTextWidth) / 2, 20, menuFontSize, WHITE);

    // Draw controls
    controls->draw();
}

void Game::drawPaused() {
    // Draw game state in background
    drawPlaying();

    // Draw pause overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, 150});

    ui->draw(state, mode);
}

void Game::drawGameOver() {
    // Draw UI
    ui->draw(state, mode);
}

void Game::drawLevelSelect() {
    ui->draw(state, mode);
}

void Game::startGame(GameMode newMode) {
    mode = newMode;
    state = GameState::PLAYING;
    score = 0;
    gameTime = 0;

    // Reset player
    delete player;
    player = new Player();

    // Clear enemies
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    // Set time based on mode
    if (mode == GameMode::TIME_CHALLENGE) {
        timeRemaining = 180.0f;  // 3 minutes
    } else if (mode == GameMode::LEVEL) {
        timeRemaining = 0;  // Will be set by level definition
    }

    audio->playButtonClickSound();
}

void Game::resetGame() {
    score = 0;
    gameTime = 0;
    currentLevel = 1;

    // Reset player
    if (player) {
        delete player;
        player = new Player();
    }

    // Clear enemies
    for (auto* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
}

void Game::spawnEnemies() {
    // Keep a minimum number of enemies
    int minEnemies = 5 + (int)(gameTime / 30.0f);
    minEnemies = (minEnemies > 30) ? 30 : minEnemies;

    if ((int)enemies.size() < minEnemies) {
        // Get camera visible bounds for spawning around player view
        Rectangle visibleArea = camera->getVisibleBounds();

        // Spawn new enemy around camera view
        int side = rand() % 4;
        Vector2 pos;
        int margin = 100;

        switch (side) {
            case 0:  // Top
                pos = {visibleArea.x + (float)(rand() % (int)visibleArea.width), visibleArea.y - margin};
                break;
            case 1:  // Bottom
                pos = {visibleArea.x + (float)(rand() % (int)visibleArea.width), visibleArea.y + visibleArea.height + margin};
                break;
            case 2:  // Left
                pos = {visibleArea.x - margin, visibleArea.y + (float)(rand() % (int)visibleArea.height)};
                break;
            case 3:  // Right
                pos = {visibleArea.x + visibleArea.width + margin, visibleArea.y + (float)(rand() % (int)visibleArea.height)};
                break;
        }

        // Clamp to world bounds
        if (pos.x < 0) pos.x = 0;
        if (pos.x > WORLD_WIDTH) pos.x = (float)WORLD_WIDTH;
        if (pos.y < 0) pos.y = 0;
        if (pos.y > WORLD_HEIGHT) pos.y = (float)WORLD_HEIGHT;

        // Determine enemy type based on player level
        EnemyType type = EnemyType::FLOATING;
        int typeRoll = rand() % 100;

        if (player->getLevel() >= 2) {
            if (typeRoll < 20) type = EnemyType::CHASING;
            else if (typeRoll < 40) type = EnemyType::BOUNCING;
            else if (typeRoll < 60) type = EnemyType::STATIONARY;
        } else {
            if (typeRoll < 30) type = EnemyType::STATIONARY;
            else if (typeRoll < 50) type = EnemyType::BOUNCING;
        }

        // Determine size based on player level
        int size = 20 + (rand() % (player->getSize() + 20));

        Enemy* enemy = new Enemy(type, pos, size);
        enemies.push_back(enemy);
    }
}

void Game::checkCollisions() {
    int playerSize = player->getSize();
    Vector2 playerPos = player->getPosition();

    auto it = enemies.begin();
    while (it != enemies.end()) {
        Enemy* enemy = *it;
        if (!enemy->isAlive()) {
            delete enemy;
            it = enemies.erase(it);
            continue;
        }

        Vector2 enemyPos = enemy->getPosition();
        int enemySize = enemy->getSize();

        // Simple AABB collision
        float dx = fabs(playerPos.x - enemyPos.x);
        float dy = fabs(playerPos.y - enemyPos.y);
        float combinedHalfSize = (playerSize + enemySize) / 2.0f;

        if (dx < combinedHalfSize && dy < combinedHalfSize) {
            if (playerSize >= enemySize) {
                // Player eats enemy
                player->addExperience(enemy->getExpValue());
                player->heal(5);
                score += enemy->getExpValue() * 10;

                // Check for level up
                int oldLevel = player->getLevel();
                player->addExperience(enemy->getExpValue());
                if (player->getLevel() > oldLevel) {
                    particles->spawnLevelUp(playerPos, player->getLevel());
                    audio->playLevelUpSound();
                } else {
                    audio->playEatSound(player->getLevel());
                }

                // Spawn particles
                particles->spawnPixelExplosion(enemyPos, enemy->getColor(), 10);
                particles->spawnTextPopup(enemyPos, "+10", {100, 255, 100, 255});

                enemy->kill();
            } else {
                // Enemy hurts player
                int damage = enemySize / 3;
                player->takeDamage(damage);
                audio->playHitSound();

                // Spawn damage number
                particles->spawnDamageNumber(playerPos, damage, false);

                // Push player back
                Vector2 pushDir = Vector2Normalize(playerPos - enemyPos);
                player->setPosition(playerPos + pushDir * 20.0f);
            }
        }

        ++it;
    }
}

// Note: Vector2Length and Vector2Normalize are defined as inline functions in game.h

