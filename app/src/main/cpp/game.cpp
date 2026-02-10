#include "game.h"
#include "player.h"
#include "enemy.h"
#include "particles.h"
#include "ui.h"
#include "audio.h"
#include "modes.h"
#include "controls.h"
#include "assets.h"
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
    , state(GameState::MENU)
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

    particles = new ParticleSystem();

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
            updatePaused();  // Similar to pause
            break;
    }

    ui->update(deltaTime);
    particles->update(deltaTime);

    // Check for pause button
    if ((GetTouchPointCount() > 0) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = (GetTouchPointCount() > 0) ? GetTouchPosition(0) : GetMousePosition();
        // Check pause button area (top right)
        if (pos.x > SCREEN_WIDTH - 100 && pos.x < SCREEN_WIDTH - 50 &&
            pos.y > 30 && pos.y < 80) {
            if (state == GameState::PLAYING) {
                state = GameState::PAUSED;
                audio->playButtonClickSound();
            } else if (state == GameState::PAUSED) {
                state = GameState::PLAYING;
                audio->playButtonClickSound();
            }
        }
    }
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
}

void Game::updateMenu() {
    // Check for menu button clicks
    if (GetTouchPointCount() > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
        int buttonWidth = 300;
        int buttonHeight = 60;
        int startY = 250;
        int gap = 80;

        if (pos.x >= SCREEN_WIDTH / 2 - buttonWidth / 2 && pos.x <= SCREEN_WIDTH / 2 + buttonWidth / 2) {
            if (pos.y >= startY && pos.y <= startY + buttonHeight) {
                // Play Endless
                startGame(GameMode::ENDLESS);
            } else if (pos.y >= startY + gap && pos.y <= startY + gap + buttonHeight) {
                // Level Mode
                state = GameState::LEVEL_SELECT;
                audio->playButtonClickSound();
                ui->resetTransition();
            } else if (pos.y >= startY + gap * 2 && pos.y <= startY + gap * 2 + buttonHeight) {
                // Time Challenge
                startGame(GameMode::TIME_CHALLENGE);
            } else if (pos.y >= startY + gap * 3 && pos.y <= startY + gap * 3 + buttonHeight) {
                // Settings
                state = GameState::SETTINGS;
                audio->playButtonClickSound();
                ui->resetTransition();
            } else if (pos.y >= startY + gap * 4 && pos.y <= startY + gap * 4 + buttonHeight) {
                // Quit - exit the game
                audio->playButtonClickSound();
                return;  // This will exit the run() loop
            }
        }
    }
}

void Game::updatePlaying() {
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
}

void Game::updatePaused() {
    // Check for resume/quit buttons
    if (GetTouchPointCount() > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
        int buttonWidth = 250;
        int buttonHeight = 50;

        if (pos.x >= SCREEN_WIDTH / 2 - buttonWidth / 2 && pos.x <= SCREEN_WIDTH / 2 + buttonWidth / 2) {
            if (pos.y >= 250 && pos.y <= 250 + buttonHeight) {
                // Resume (only when actually paused)
                if (state == GameState::PAUSED) {
                    state = GameState::PLAYING;
                    audio->playButtonClickSound();
                }
            } else if (pos.y >= 320 && pos.y <= 320 + buttonHeight) {
                // Quit to menu
                state = GameState::MENU;
                audio->playButtonClickSound();
                resetGame();
                ui->resetTransition();
            }
        }

        // Handle Settings back button (at y=500, height=50)
        if (state == GameState::SETTINGS) {
            if (pos.x >= SCREEN_WIDTH / 2 - 100 && pos.x <= SCREEN_WIDTH / 2 + 100 &&
                pos.y >= 500 && pos.y <= 550) {
                state = GameState::MENU;
                audio->playButtonClickSound();
                ui->resetTransition();
            }
        }
    }
}

void Game::updateGameOver() {
    // Check for try again/main menu buttons
    if (GetTouchPointCount() > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();
        int buttonWidth = 250;
        int buttonHeight = 50;

        if (pos.x >= SCREEN_WIDTH / 2 - buttonWidth / 2 && pos.x <= SCREEN_WIDTH / 2 + buttonWidth / 2) {
            if (pos.y >= 350 && pos.y <= 350 + buttonHeight) {
                // Try Again
                startGame(mode);
            } else if (pos.y >= 420 && pos.y <= 420 + buttonHeight) {
                // Main Menu
                state = GameState::MENU;
                audio->playButtonClickSound();
                resetGame();
                ui->resetTransition();
            }
        }
    }
}

void Game::updateLevelSelect() {
    // Check for level selection
    if (GetTouchPointCount() > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition();

        // Check back button area
        if (pos.x >= SCREEN_WIDTH / 2 - 100 && pos.x <= SCREEN_WIDTH / 2 + 100 &&
            pos.y >= 500 && pos.y <= 550) {
            state = GameState::MENU;
            audio->playButtonClickSound();
            ui->resetTransition();
            return;
        }

        // Check level buttons
        int buttonSize = 80;
        int startX = (SCREEN_WIDTH - 5 * (buttonSize + 20)) / 2 + 10;
        int startY = 150;

        for (int i = 0; i < 10; i++) {
            int row = i / 5;
            int col = i % 5;
            int x = startX + col * (buttonSize + 20);
            int y = startY + row * (buttonSize + 20);

            if (pos.x >= x && pos.x <= x + buttonSize && pos.y >= y && pos.y <= y + buttonSize) {
                currentLevel = i + 1;
                startGame(GameMode::LEVEL);
                return;
            }
        }
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
    // Draw player
    player->draw();

    // Draw enemies
    for (auto* enemy : enemies) {
        enemy->draw();
    }

    // Draw UI
    ui->drawHUD(player);
    ui->drawScore(score);

    if (mode == GameMode::TIME_CHALLENGE || mode == GameMode::LEVEL) {
        ui->drawTimer(timeRemaining);
    }

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
        // Spawn new enemy
        int side = rand() % 4;
        Vector2 pos;
        int margin = 50;

        switch (side) {
            case 0:  // Top
                pos = {(float)(rand() % SCREEN_WIDTH), (float)-margin};
                break;
            case 1:  // Bottom
                pos = {(float)(rand() % SCREEN_WIDTH), (float)(SCREEN_HEIGHT + margin)};
                break;
            case 2:  // Left
                pos = {(float)-margin, (float)(rand() % SCREEN_HEIGHT)};
                break;
            case 3:  // Right
                pos = {(float)(SCREEN_WIDTH + margin), (float)(rand() % SCREEN_HEIGHT)};
                break;
        }

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

