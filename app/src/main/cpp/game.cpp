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
    // Set initial volume from UI
    audio->setMasterVolume(ui->getMasterVolume());

    controls = new ControlSystem();
    controls->init();

    ui = new UIManager();
    // Initialize UI with fonts (if loaded)
    ui->init(&assets->GetPixelFont(), &assets->GetSmallFont());
    // Sync control mode with UI
    ui->setControlMode(controlMode);

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

    // Update player - pass player position for touch follow mode
    Vector2 input = controls->getInputVector(player->getPosition());
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

    // Check for pause button click (top right corner)
    if ((GetTouchPointCount() > 0) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = (GetTouchPointCount() > 0) ? GetTouchPosition(0) : GetMousePosition();
        // Pause button area: x=SCREEN_WIDTH-100 to SCREEN_WIDTH-50, y=30 to 80
        if (pos.x >= SCREEN_WIDTH - 100 && pos.x <= SCREEN_WIDTH - 50 &&
            pos.y >= 30 && pos.y <= 80) {
            state = GameState::PAUSED;
            audio->playButtonClickSound();
        }
    }
}

void Game::updatePaused() {
    // Store current state before any state changes
    if (state != GameState::SETTINGS) {
        previousState = state;
    }
    
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
                ui->setControlMode(controlMode);  // Update UI display
                controls->setMode(controlMode);   // Update controls
                break;
            case 3:  // Back (was previous case 3)
                state = previousState;
                ui->resetTransition();
                break;
            case 4:  // Toggle Mute
                ui->toggleMute();
                audio->setMuted(ui->isMuted());
                audio->setMasterVolume(ui->getMasterVolume());
                break;
            case 5:  // Back (new position)
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
    // Don't draw controls in menu state
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

    // Draw controls (includes pause button)
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
        // Use player position directly for spawning
        Vector2 playerPos = player->getPosition();

        // Spawn new enemy around player with safe distance
        int side = rand() % 4;
        Vector2 pos;
        float minSpawnDist = player->getSize() + 100.0f;  // Minimum safe distance
        float maxSpawnDist = minSpawnDist + 200.0f;     // Maximum spawn distance
        float spawnDist = minSpawnDist + (float)(rand() % (int)(maxSpawnDist - minSpawnDist));

        switch (side) {
            case 0:  // Top
                pos = {playerPos.x + (float)(rand() % 200 - 100), playerPos.y - spawnDist};
                break;
            case 1:  // Bottom
                pos = {playerPos.x + (float)(rand() % 200 - 100), playerPos.y + spawnDist};
                break;
            case 2:  // Left
                pos = {playerPos.x - spawnDist, playerPos.y + (float)(rand() % 200 - 100)};
                break;
            case 3:  // Right
                pos = {playerPos.x + spawnDist, playerPos.y + (float)(rand() % 200 - 100)};
                break;
        }

        // Clamp to world bounds
        if (pos.x < 100) pos.x = 100;
        if (pos.x > WORLD_WIDTH - 100) pos.x = (float)WORLD_WIDTH - 100;
        if (pos.y < 100) pos.y = 100;
        if (pos.y > WORLD_HEIGHT - 100) pos.y = (float)WORLD_HEIGHT - 100;

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

        // Determine size - ensure enemies are not too small for current player level
        int playerSize = player->getSize();
        int minEnemySize = playerSize - 10;
        int maxEnemySize = playerSize + 30;
        if (minEnemySize < 15) minEnemySize = 15;
        int size = minEnemySize + (rand() % (maxEnemySize - minEnemySize));

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

