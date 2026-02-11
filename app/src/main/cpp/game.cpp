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
#include "bullet.h"
#include "skills.h"
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
    , skillManager(nullptr)
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

    // Generate and load space background texture
    backgroundTexture = assets->GeneratePixelBackground();

    audio = new AudioManager();
    audio->init();

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

    // Initialize skill manager
    skillManager = new SkillManager();
    skillManager->init();

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
    audio->updateMusic();  // Update music streaming
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

    // Clear bullets
    for (auto* bullet : bullets) {
        delete bullet;
    }
    bullets.clear();

    // Unload background texture
    if (backgroundTexture.id != 0) {
        UnloadTexture(backgroundTexture);
    }

    // Delete managers
    delete player;
    delete particles;
    delete ui;
    delete audio;
    delete controls;
    delete assets;
    delete camera;
    delete skillManager;
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

    // Update bullets
    for (auto* bullet : bullets) {
        bullet->update(deltaTime);
    }

    // Update skill manager
    skillManager->update(deltaTime);

    // Check collisions
    checkCollisions();

    // Spawn enemies
    spawnEnemies();

    // Update time remaining for time challenge mode
    if (mode == GameMode::TIME_CHALLENGE || mode == GameMode::LEVEL) {
        timeRemaining -= deltaTime;
        if (timeRemaining <= 0) {
            timeRemaining = 0;
            state = GameState::GAME_OVER;
            audio->playDeathSound();
        }
    }

    // Check game over
    if (player->getHealth() <= 0) {
        state = GameState::GAME_OVER;
        audio->playDeathSound();
    }

    // Update game time
    gameTime += deltaTime;

    // Update UI
    ui->update(deltaTime);

    // Check for skill button clicks (bottom-right corner)
    int touchCount = GetTouchPointCount();
    if (touchCount > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = (touchCount > 0) ? GetTouchPosition(0) : GetMousePosition();

        // Skill button area: bottom-right
        float buttonSize = 60.0f;
        float startX = SCREEN_WIDTH - 280.0f;
        float startY = SCREEN_HEIGHT - 80.0f;
        float spacing = 70.0f;

        for (int i = 0; i < 4; i++) {
            float x = startX + i * spacing;
            if (pos.x >= x && pos.x <= x + buttonSize &&
                pos.y >= startY && pos.y <= startY + buttonSize) {
                // Skill button clicked
                SkillType skillType = (SkillType)i;
                if (skillManager->canUseSkill(skillType)) {
                    Vector2 facingDir = player->getFacingDirection();
                    int playerHP = player->getHealth();

                    if (skillType == SkillType::BLINK) {
                        // Handle blink - move player
                        float blinkDist = player->getSize() * 5.0f;
                        Vector2 newPos = {
                            player->getPosition().x + facingDir.x * blinkDist,
                            player->getPosition().y + facingDir.y * blinkDist
                        };
                        // Clamp to world bounds
                        if (newPos.x < player->getSize()) newPos.x = player->getSize();
                        if (newPos.x > WORLD_WIDTH - player->getSize()) newPos.x = WORLD_WIDTH - player->getSize();
                        if (newPos.y < player->getSize()) newPos.y = player->getSize();
                        if (newPos.y > WORLD_HEIGHT - player->getSize()) newPos.y = WORLD_HEIGHT - player->getSize();
                        player->setPosition(newPos);
                        skillManager->useSkill(skillType, newPos, facingDir, player->getSize(), playerHP);
                        audio->playBlinkSound();
                    } else if (skillType == SkillType::SHOOT) {
                        // Handle shoot - create bullet and consume HP
                        int hpCost = 20;
                        int currentHP = player->getHealth();
                        if (currentHP > hpCost) {
                            player->takeDamage(hpCost);
                            int damage = hpCost * 3;
                            Bullet* bullet = new Bullet(player->getPosition(), facingDir, damage, 0);
                            bullets.push_back(bullet);
                            skillManager->useSkill(skillType, player->getPosition(), facingDir, player->getSize(), currentHP);
                            audio->playShootSound();
                        }
                    } else if (skillType == SkillType::SHIELD) {
                        // Handle shield - set shield duration based on player level
                        int playerLevel = player->getLevel();
                        float shieldDuration = 1.0f + (playerLevel - 1) * 1.0f;  // 1-15 seconds
                        if (shieldDuration > 15.0f) shieldDuration = 15.0f;
                        skillManager->setShieldDuration(shieldDuration);
                        skillManager->useSkill(skillType, player->getPosition(), facingDir, player->getSize(), playerHP);
                        audio->playShieldSound();
                    } else {
                        skillManager->useSkill(skillType, player->getPosition(), facingDir, player->getSize(), playerHP);
                        audio->playRotateSound();
                    }
                }
                break;  // Only handle one button click at a time
            }
        }
    }

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
    int logsSelection = ui->getLogsSelection();

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
            case 6:  // View Logs
                ui->setCurrentPanel(MenuPanel::LOGS);
                break;
        }

        ui->clearSelections();
    }

    // Handle logs panel back button
    if (logsSelection >= 0) {
        audio->playButtonClickSound();
        ui->setCurrentPanel(MenuPanel::SETTINGS);  // Return to settings
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
    // Draw space-themed background with stars
    if (backgroundTexture.id != 0) {
        // Draw the space background texture to fill the screen
        DrawTextureRec(backgroundTexture,
                       (Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT},
                       (Vector2){0, 0},
                       WHITE);
    } else {
        // Fallback: draw simple dark background
        ClearBackground({15, 15, 30, 255});
    }
}

void Game::drawMenu() {
    ui->draw(state, mode);
    // Don't draw controls in menu state
}

void Game::drawPlaying() {
    // Apply camera for world rendering
    camera->apply();

    // Draw world map border (visible boundary around the play area)
    float borderWidth = 10.0f;
    Color borderColor = {255, 100, 100, 200};  // Red border
    Color borderOutlineColor = {255, 150, 150, 255};  // Lighter outline

    // Draw main border rectangle
    DrawRectangleLinesEx((Rectangle){0, 0, WORLD_WIDTH, WORLD_HEIGHT}, borderWidth, borderColor);
    DrawRectangleLinesEx((Rectangle){0, 0, WORLD_WIDTH, WORLD_HEIGHT}, borderWidth / 2, borderOutlineColor);

    // Draw corner markers for additional visibility
    float cornerSize = 100.0f;
    DrawRectangle(0, 0, cornerSize, borderWidth, borderOutlineColor);  // Top-left
    DrawRectangle(0, 0, borderWidth, cornerSize, borderOutlineColor);
    DrawRectangle(WORLD_WIDTH - cornerSize, 0, cornerSize, borderWidth, borderOutlineColor);  // Top-right
    DrawRectangle(WORLD_WIDTH - borderWidth, 0, borderWidth, cornerSize, borderOutlineColor);
    DrawRectangle(0, WORLD_HEIGHT - borderWidth, cornerSize, borderWidth, borderOutlineColor);  // Bottom-left
    DrawRectangle(0, WORLD_HEIGHT - cornerSize, borderWidth, cornerSize, borderOutlineColor);
    DrawRectangle(WORLD_WIDTH - cornerSize, WORLD_HEIGHT - borderWidth, cornerSize, borderWidth, borderOutlineColor);  // Bottom-right
    DrawRectangle(WORLD_WIDTH - borderWidth, WORLD_HEIGHT - cornerSize, borderWidth, cornerSize, borderOutlineColor);

    // Draw blink effect (flash trail)
    if (skillManager->getBlinkTimer() > 0) {
        Vector2 blinkFrom = skillManager->getBlinkFromPos();
        Vector2 blinkTo = skillManager->getBlinkToPos();
        float blinkAlpha = skillManager->getBlinkTimer() / 0.3f;  // Fade out

        // Draw line from old position to new position
        DrawLineEx(blinkFrom, blinkTo, 10.0f, {255, 255, 100, (unsigned char)(200 * blinkAlpha)});
        DrawCircleV(blinkFrom, 30.0f, {255, 255, 100, (unsigned char)(100 * blinkAlpha)});
        DrawCircleV(blinkTo, 40.0f, {255, 255, 150, (unsigned char)(150 * blinkAlpha)});
    }

    // Draw shield if active
    if (skillManager->isShieldActive()) {
        Vector2 shieldPos = skillManager->getShieldPosition();
        Vector2 shieldDir = skillManager->getShieldDirection();
        int shieldLevel = skillManager->getShieldLevel();

        // Draw arc shield (45° angle)
        float shieldRadius = 80.0f;
        float startAngle = atan2f(shieldDir.y, shieldDir.x) - 22.5f * DEG2RAD;
        float endAngle = atan2f(shieldDir.y, shieldDir.x) + 22.5f * DEG2RAD;

        // Pulsing shield effect
        float pulse = sinf(GetTime() * 8.0f) * 0.2f + 1.0f;
        Color shieldColor = {
            (unsigned char)(100 * pulse),
            (unsigned char)(255 * pulse),
            (unsigned char)(100 * pulse),
            180
        };
        Color shieldBorderColor = {
            (unsigned char)(150 * pulse),
            255,
            (unsigned char)(150 * pulse),
            220
        };

        DrawCircleSector((Vector2){shieldPos.x, shieldPos.y}, shieldRadius * pulse, startAngle, endAngle, 30, shieldColor);
        DrawCircleSectorLines((Vector2){shieldPos.x, shieldPos.y}, shieldRadius * pulse, startAngle, endAngle, 30, shieldBorderColor);
    }

    // Draw rotate effect (spinning particles around player)
    if (skillManager->isRotating()) {
        Vector2 playerPos = player->getPosition();
        float rotateTimer = skillManager->getRotateTimer();
        float rotation = GetTime() * 10.0f;  // Spinning animation
        int numParticles = 8;
        float orbitRadius = player->getSize() + 30.0f;

        for (int i = 0; i < numParticles; i++) {
            float angle = rotation + (i * 2.0f * PI / numParticles);
            Vector2 particlePos = {
                playerPos.x + cosf(angle) * orbitRadius,
                playerPos.y + sinf(angle) * orbitRadius
            };

            float size = 5.0f + sinf(GetTime() * 5.0f + i) * 3.0f;
            Color particleColor = {
                255,
                (unsigned char)(150 + i * 15),
                0,
                (unsigned char)(200 * (rotateTimer / 2.0f))
            };
            DrawCircleV(particlePos, size, particleColor);
        }
    }

    // Draw player
    player->draw();

    // Draw enemies
    for (auto* enemy : enemies) {
        enemy->draw();
    }

    // Draw bullets
    for (auto* bullet : bullets) {
        bullet->draw();
    }

    // End camera mode (switch back to screen space for UI)
    camera->end();

    // Draw UI (in screen space)
    ui->drawHUD(player);
    ui->drawScore(score);

    if (mode == GameMode::TIME_CHALLENGE || mode == GameMode::LEVEL) {
        ui->drawTimer(timeRemaining);
    }

    // Draw FPS counter (top-left corner)
    int fps = (int)GetFPS();
    char fpsText[32];
    sprintf(fpsText, "FPS: %d", fps);
    DrawText(fpsText, 10, 10, 16, {255, 255, 255, 200});

    // Draw skill buttons
    skillManager->draw();

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
    // Keep a minimum number of enemies - increased significantly
    int minEnemies = 20 + (int)(gameTime / 10.0f);  // More enemies, faster increase
    minEnemies = (minEnemies > 100) ? 100 : minEnemies;  // Higher cap

    if ((int)enemies.size() < minEnemies) {
        // Spawn multiple enemies at once for better gameplay
        int spawnCount = 3 + (int)(gameTime / 60.0f);  // Spawn in groups
        if (spawnCount > 10) spawnCount = 10;

        for (int i = 0; i < spawnCount && (int)enemies.size() < minEnemies; i++) {
            // Use player position directly for spawning
            Vector2 playerPos = player->getPosition();

            // Spawn new enemy around player with safe distance
            int side = rand() % 4;
            Vector2 pos;
            float minSpawnDist = player->getSize() + 150.0f;  // Increased safe distance
            float maxSpawnDist = minSpawnDist + 400.0f;     // Larger spawn area
            float spawnDist = minSpawnDist + (float)(rand() % (int)(maxSpawnDist - minSpawnDist));

            switch (side) {
                case 0:  // Top
                    pos = {playerPos.x + (float)(rand() % 400 - 200), playerPos.y - spawnDist};
                    break;
                case 1:  // Bottom
                    pos = {playerPos.x + (float)(rand() % 400 - 200), playerPos.y + spawnDist};
                    break;
                case 2:  // Left
                    pos = {playerPos.x - spawnDist, playerPos.y + (float)(rand() % 400 - 200)};
                    break;
                case 3:  // Right
                    pos = {playerPos.x + spawnDist, playerPos.y + (float)(rand() % 400 - 200)};
                    break;
            }

            // Clamp to world bounds
            if (pos.x < 100) pos.x = 100;
            if (pos.x > WORLD_WIDTH - 100) pos.x = (float)WORLD_WIDTH - 100;
            if (pos.y < 100) pos.y = 100;
            if (pos.y > WORLD_HEIGHT - 100) pos.y = (float)WORLD_HEIGHT - 100;

            // Determine enemy type - add more small food pellets (increased from 30% to 50%)
            EnemyType type = EnemyType::FLOATING;
            int typeRoll = rand() % 100;
            int playerLevel = player->getLevel();

            // 50% chance for small food pellets (always smaller than player) - increased for easier leveling
            if (typeRoll < 50) {
                type = EnemyType::STATIONARY;  // Food pellets are stationary
            } else if (playerLevel >= 2) {
                if (typeRoll < 65) type = EnemyType::CHASING;
                else if (typeRoll < 80) type = EnemyType::BOUNCING;
                else type = EnemyType::STATIONARY;
            } else {
                if (typeRoll < 60) type = EnemyType::STATIONARY;
                else if (typeRoll < 75) type = EnemyType::BOUNCING;
            }

            // Determine size - mix of food pellets and dangerous enemies
            int playerSize = player->getSize();
            int minEnemySize, maxEnemySize;

            if (typeRoll < 30) {
                // Food pellets - smaller than player
                minEnemySize = 10;
                maxEnemySize = playerSize - 5;
                if (maxEnemySize < 10) maxEnemySize = 10;
            } else {
                // Dangerous enemies - can be larger or smaller
                minEnemySize = playerSize - 15;
                maxEnemySize = playerSize + 40;
                if (minEnemySize < 15) minEnemySize = 15;
            }

            int size = minEnemySize + (rand() % (maxEnemySize - minEnemySize));

            Enemy* enemy = new Enemy(type, pos, size);
            enemies.push_back(enemy);
        }
    }
}

void Game::checkCollisions() {
    int playerSize = player->getSize();
    Vector2 playerPos = player->getPosition();

    // Check bullet-enemy collisions
    auto bulletIt = bullets.begin();
    while (bulletIt != bullets.end()) {
        Bullet* bullet = *bulletIt;
        if (!bullet->isAlive()) {
            delete bullet;
            bulletIt = bullets.erase(bulletIt);
            continue;
        }

        Vector2 bulletPos = bullet->getPosition();
        int bulletSize = bullet->getSize();

        // Check collision with enemies
        bool bulletHit = false;
        for (auto* enemy : enemies) {
            if (!enemy->isAlive()) continue;

            Vector2 enemyPos = enemy->getPosition();
            int enemySize = enemy->getSize();

            float dx = fabs(bulletPos.x - enemyPos.x);
            float dy = fabs(bulletPos.y - enemyPos.y);
            float combinedHalfSize = (bulletSize + enemySize) / 2.0f;

            if (dx < combinedHalfSize && dy < combinedHalfSize) {
                // Bullet hit enemy
                int damage = bullet->getDamage();
                enemy->takeDamage(damage);
                bullet->kill();
                bulletHit = true;

                // Spawn hit effect
                particles->spawnPixelExplosion(bulletPos, {255, 255, 0, 255}, 5);
                particles->spawnDamageNumber(enemyPos, damage, true);

                if (!enemy->isAlive()) {
                    score += damage * 5;
                    player->addExperience(damage / 2);
                }
                break;
            }
        }

        if (!bulletHit) {
            ++bulletIt;
        } else {
            // Bullet was removed, iterator is already at next element
        }
    }

    // Clean up dead bullets
    bulletIt = bullets.begin();
    while (bulletIt != bullets.end()) {
        if (!(*bulletIt)->isAlive()) {
            delete *bulletIt;
            bulletIt = bullets.erase(bulletIt);
        } else {
            ++bulletIt;
        }
    }

    // Check player-enemy collisions
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
                // Player eats enemy - doubled experience gain for faster leveling
                int expGained = enemy->getExpValue() * 2;
                player->addExperience(expGained);
                player->heal(5);
                score += enemy->getExpValue() * 10;

                // Check for level up
                int oldLevel = player->getLevel();
                player->addExperience(expGained);
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
                // Enemy hurts player - check for rotation skill (damage reduction)
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

