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
#include "userManager.h"
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
    , userManager(nullptr)
    , state(GameState::MENU)
    , previousState(GameState::MENU)
    , mode(GameMode::ENDLESS)
    , controlMode(ControlMode::VIRTUAL_JOYSTICK)
    , score(0)
    , currentLevel(1)
    , timeRemaining(0)
    , deltaTime(0)
    , gameTime(0)
    , nameInputBuffer{0}  // Initialize empty string
    , hasRecentSave(false)
    , timeSinceLastSave(0)
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

    // Initialize user manager
    userManager = new UserManager();
    userManager->init();

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
        case GameState::USER_MENU:
            updateUserMenu();
            break;
        case GameState::NAME_INPUT:
            updateNameInput();
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
        case GameState::USER_MENU:
            drawUserMenu();
            break;
        case GameState::NAME_INPUT:
            drawNameInput();
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

    // Update player - physics-based movement
    Vector2 input = controls->getInputVector(player->getPosition());
    player->applyJoystickInput(input);
    player->update(deltaTime, bullets);

    // Update enemies (with bullet shooting for FLOATING types and all enemies list)
    for (auto* enemy : enemies) {
        enemy->update(deltaTime, player->getPosition(), bullets, enemies);
    }
    
    // Rigid body collisions between enemies
    for (size_t i = 0; i < enemies.size(); i++) {
        for (size_t j = i + 1; j < enemies.size(); j++) {
            Enemy* e1 = enemies[i];
            Enemy* e2 = enemies[j];
            if (!e1->isAlive() || !e2->isAlive()) continue;
            
            // Check collision
            float dx = fabs(e1->getPosition().x - e2->getPosition().x);
            float dy = fabs(e1->getPosition().y - e2->getPosition().y);
            float combinedHalfSize = (e1->getSize() + e2->getSize()) / 2.0f;
            
            if (dx < combinedHalfSize && dy < combinedHalfSize) {
                // Apply rigid body collision
                Vector2 normal = Vector2Normalize(e1->getPosition() - e2->getPosition());
                e1->applyRigidBodyCollision(e2);
                
                // BOUNCING type special: deal damage and push away strongly
                if (e1->getType() == EnemyType::BOUNCING) {
                    e1->applyBouncingDamage(e2);
                }
                if (e2->getType() == EnemyType::BOUNCING) {
                    e2->applyBouncingDamage(e1);
                }
            }
        }
    }

    // Update bullets
    for (auto* bullet : bullets) {
        bullet->update(deltaTime);
    }

    // Update skill manager
    skillManager->update(deltaTime);
    
    // Process shield interactions (convex reflection, concave acceleration)
    skillManager->processShieldInteractions(player, enemies);

    // Check collisions
    checkCollisions();

    // Spawn enemies
    spawnEnemies();

    // Update time remaining for time challenge mode
    // For LEVEL mode, only check timeout if timeRemaining > 0 (has time limit)
    if (mode == GameMode::TIME_CHALLENGE) {
        timeRemaining -= deltaTime;
        if (timeRemaining <= 0) {
            timeRemaining = 0;
            state = GameState::GAME_OVER;
            audio->playDeathSound();
        }
    } else if (mode == GameMode::LEVEL && timeRemaining > 0) {
        // Only check timeout for levels that have a time limit
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
    timeSinceLastSave += deltaTime;

    // Update UI
    ui->update(deltaTime);

    // Check for skill button clicks (bottom-right corner)
    // CRITICAL FIX: Check ALL touch points, not just the first one
    // This allows using joystick (touch 0) and skills (touch 1) simultaneously
    int touchCount = GetTouchPointCount();

    // Quick save: Check for F5 key or Esc key
    int quickSaveKey = IsKeyPressed(KEY_F5) || IsKeyPressed(KEY_ESCAPE);
    if (quickSaveKey && !hasRecentSave) {
        quickSave();
    }

    // Skill button area: bottom-right
    float buttonSize = 60.0f;
    float startX = SCREEN_WIDTH - 280.0f;
    float startY = SCREEN_HEIGHT - 80.0f;
    float spacing = 70.0f;

    // Check each touch point for skill button clicks
    for (int t = 0; t < touchCount; t++) {
        Vector2 pos = GetTouchPosition(t);

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
                break;  // Only handle one button click per touch point
            }
        }
    }

    // Also check mouse clicks for testing on desktop
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = GetMousePosition();
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
                        float blinkDist = player->getSize() * 5.0f;
                        Vector2 newPos = {
                            player->getPosition().x + facingDir.x * blinkDist,
                            player->getPosition().y + facingDir.y * blinkDist
                        };
                        if (newPos.x < player->getSize()) newPos.x = player->getSize();
                        if (newPos.x > WORLD_WIDTH - player->getSize()) newPos.x = WORLD_WIDTH - player->getSize();
                        if (newPos.y < player->getSize()) newPos.y = player->getSize();
                        if (newPos.y > WORLD_HEIGHT - player->getSize()) newPos.y = WORLD_HEIGHT - player->getSize();
                        player->setPosition(newPos);
                        skillManager->useSkill(skillType, newPos, facingDir, player->getSize(), playerHP);
                        audio->playBlinkSound();
                    } else if (skillType == SkillType::SHOOT) {
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
                        int playerLevel = player->getLevel();
                        float shieldDuration = 1.0f + (playerLevel - 1) * 1.0f;
                        if (shieldDuration > 15.0f) shieldDuration = 15.0f;
                        skillManager->setShieldDuration(shieldDuration);
                        skillManager->useSkill(skillType, player->getPosition(), facingDir, player->getSize(), playerHP);
                        audio->playShieldSound();
                    } else {
                        skillManager->useSkill(skillType, player->getPosition(), facingDir, player->getSize(), playerHP);
                        audio->playRotateSound();
                    }
                }
                break;
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
            case 2:  // Toggle Mute
                ui->toggleMute();
                audio->setMuted(ui->isMuted());
                audio->setMasterVolume(ui->getMasterVolume());
                break;
            case 3:  // View Logs
                ui->setCurrentPanel(MenuPanel::LOGS);
                break;
            case 4:  // Back
                state = previousState;
                ui->resetTransition();
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

        // Draw arc shield (45° angle) - DrawCircleSector uses degrees
        float shieldRadius = 80.0f;
        float baseAngle = atan2f(shieldDir.y, shieldDir.x) * RAD2DEG;  // Convert rad to deg
        float startAngle = baseAngle - 22.5f;
        float endAngle = baseAngle + 22.5f;

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

    if (mode == GameMode::TIME_CHALLENGE) {
        ui->drawTimer(timeRemaining);
    } else if (mode == GameMode::LEVEL && timeRemaining > 0) {
        // Only draw timer for levels that have a time limit
        ui->drawTimer(timeRemaining);
    }

    // Draw FPS counter (top-right corner, next to pause button)
    int fps = (int)GetFPS();
    char fpsText[32];
    sprintf(fpsText, "FPS: %d", fps);
    DrawText(fpsText, SCREEN_WIDTH - 160, 45, 16, {255, 255, 255, 200});

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
        // CRITICAL FIX: Load time limit from level definition
        // Level time limits: 0=no limit, 120=2min, 180=3min, 240=4min, 300=5min
        switch (currentLevel) {
            case 1: timeRemaining = 0; break;     // No limit
            case 2: timeRemaining = 0; break;     // No limit
            case 3: timeRemaining = 120; break;   // 2 minutes
            case 4: timeRemaining = 0; break;     // No limit
            case 5: timeRemaining = 180; break;   // 3 minutes
            case 6: timeRemaining = 0; break;     // No limit
            case 7: timeRemaining = 240; break;   // 4 minutes
            case 8: timeRemaining = 0; break;     // No limit
            case 9: timeRemaining = 300; break;   // 5 minutes
            case 10: timeRemaining = 180; break;  // 3 minutes
            default: timeRemaining = 0; break;    // No limit
        }
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
    // Keep a minimum number of enemies - 4x spawn rate
    int minEnemies = 80 + (int)(gameTime / 2.5f);  // 4x base, 4x faster increase
    minEnemies = (minEnemies > 400) ? 400 : minEnemies;  // Higher cap

    if ((int)enemies.size() < minEnemies) {
        // Spawn multiple enemies at once - 4x spawn count
        int spawnCount = 12 + (int)(gameTime / 15.0f);  // 4x spawn groups
        if (spawnCount > 40) spawnCount = 40;

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

            // Determine enemy type - all 4 types spawn randomly
            EnemyType type = EnemyType::FLOATING;
            int typeRoll = rand() % 100;

            // All 4 types have roughly equal chance (25% each)
            if (typeRoll < 25) {
                type = EnemyType::FLOATING;
            } else if (typeRoll < 50) {
                type = EnemyType::CHASING;
            } else if (typeRoll < 75) {
                type = EnemyType::STATIONARY;
            } else {
                type = EnemyType::BOUNCING;
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

        // Check collision with enemies (player bullets only, playerId >= 0)
        bool bulletHit = false;
        if (bullet->getPlayerId() >= 0) {
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
        }
        
        // Check collision with player (enemy bullets only, playerId < 0)
        if (!bulletHit && bullet->getPlayerId() < 0) {
            float dx = fabs(bulletPos.x - playerPos.x);
            float dy = fabs(bulletPos.y - playerPos.y);
            float combinedHalfSize = (bulletSize + playerSize) / 2.0f;
            
            if (dx < combinedHalfSize && dy < combinedHalfSize) {
                // Enemy bullet hit player
                int damage = bullet->getDamage();
                player->takeDamage(damage);
                bullet->kill();
                bulletHit = true;
                
                // Spawn hit effect
                particles->spawnPixelExplosion(bulletPos, {255, 100, 100, 255}, 5);
                particles->spawnDamageNumber(playerPos, damage, false);
                audio->playHitSound();
            }
        }

        if (!bulletHit) {
            ++bulletIt;
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

    // Check shield-enemy collisions (shield blocks enemies)
    if (skillManager->isShieldActive()) {
        Vector2 shieldPos = skillManager->getShieldPosition();
        Vector2 shieldDir = skillManager->getShieldDirection();
        float shieldRadius = 80.0f;
        float baseAngle = atan2f(shieldDir.y, shieldDir.x) * RAD2DEG;
        
        for (auto* enemy : enemies) {
            if (!enemy->isAlive()) continue;
            
            Vector2 enemyPos = enemy->getPosition();
            int enemySize = enemy->getSize();
            
            // Calculate distance from enemy to shield center
            float dist = Vector2Length(enemyPos - shieldPos);
            float combinedRadius = shieldRadius + enemySize / 2.0f;
            
            if (dist < combinedRadius) {
                // Calculate angle from shield center to enemy
                float angleToEnemy = atan2f(enemyPos.y - shieldPos.y, enemyPos.x - shieldPos.x) * RAD2DEG;
                
                // Normalize angles to -180 to 180 range for comparison
                float angleDiff = angleToEnemy - baseAngle;
                while (angleDiff > 180) angleDiff -= 360;
                while (angleDiff < -180) angleDiff += 360;
                
                // Check if enemy is within 45 degree shield arc (±22.5 degrees)
                if (fabs(angleDiff) <= 22.5f) {
                    // Enemy hit the shield - push it back
                    Vector2 pushDir = Vector2Normalize(enemyPos - shieldPos);
                    enemy->setPosition(shieldPos + pushDir * (shieldRadius + enemySize / 2.0f + 5.0f));
                    
                    // Deal damage to enemy (shield level * 10)
                    int shieldDamage = skillManager->getShieldLevel() * 10;
                    enemy->takeDamage(shieldDamage);
                    
                    // Spawn hit effect
                    particles->spawnPixelExplosion(enemyPos, {100, 255, 100, 255}, 5);
                    particles->spawnDamageNumber(enemyPos, shieldDamage, true);
                    
                    // Play shield hit sound
                    audio->playHitSound();
                }
            }
        }
    }

    // Check player-enemy collisions with rigid body physics
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

        // AABB collision check
        float dx = fabs(playerPos.x - enemyPos.x);
        float dy = fabs(playerPos.y - enemyPos.y);
        float combinedHalfSize = (playerSize + enemySize) / 2.0f;

        if (dx < combinedHalfSize && dy < combinedHalfSize) {
            // Check if player can eat enemy
            bool canPlayerEat = (playerSize > enemySize) || 
                                ((enemy->getType() == EnemyType::CHASING || 
                                  enemy->getType() == EnemyType::FLOATING) && 
                                 enemy->isVulnerable());
            
            // Check if enemy can eat player (only if enemy is significantly bigger)
            bool canEnemyEat = (enemySize >= playerSize * 1.5f) && 
                               (enemy->getType() != EnemyType::STATIONARY);
            
            if (canPlayerEat && !canEnemyEat) {
                // Player eats enemy - grow by area
                player->growByArea(enemySize);
                
                // Gain bullet skill if eating FLOATING enemy
                if (enemy->getType() == EnemyType::FLOATING && !player->hasBulletSkill()) {
                    player->enableBulletSkill();
                    particles->spawnTextPopup(playerPos, "BULLET SKILL!", {255, 255, 0, 255});
                }
                
                // Experience gain
                int expGained = enemy->getExpValue() * 2;
                player->addExperience(expGained);
                player->heal(5);
                score += enemy->getExpValue() * 10;

                // Check for level up
                int oldLevel = player->getLevel();
                if (player->getLevel() > oldLevel) {
                    particles->spawnLevelUp(playerPos, player->getLevel());
                    audio->playLevelUpSound();
                }

                // Check for level completion and unlock next level
                if (mode == GameMode::LEVEL) {
                    LevelDefinition level = modeManager->getCurrentLevelDef();
                    int currentLevel = player->getLevel();

                    // Check if reached target score and level
                    if (score >= level.targetScore && currentLevel >= level.targetLevel) {
                        // Level complete! Unlock next level
                        if (currentLevel < 10) {
                            // Unlock next level
                            modeManager->nextLevel();
                            particles->spawnTextPopup(playerPos,
                                TextFormat("LEVEL %d COMPLETE!", currentLevel));
                            audio->playLevelUpSound();

                            // Update user stats
                            User* user = userManager->getCurrentUser();
                            if (user && user->maxLevelUnlocked < currentLevel) {
                                user->maxLevelUnlocked = currentLevel;
                            }
                        }
                    }
                }

                audio->playEatSound(player->getLevel());

                // Spawn particles
                particles->spawnPixelExplosion(enemyPos, enemy->getColor(), 10);
                particles->spawnTextPopup(enemyPos, "+SIZE", {100, 255, 100, 255});

                enemy->kill();
            } else if (canEnemyEat) {
                // Enemy eats player - game over
                player->takeDamage(player->getHealth());  // Kill player
                particles->spawnPixelExplosion(playerPos, {255, 0, 0, 255}, 20);
                audio->playDeathSound();
            } else {
                // Rigid body collision - both survive but bounce off each other
                Vector2 normal = Vector2Normalize(playerPos - enemyPos);
                player->applyRigidBodyCollision(enemy->getMass(), enemy->getVelocity(), normal);
                Vector2 negNormal = (Vector2){-normal.x, -normal.y};
                enemy->applyRigidBodyCollision(player->getMass(), player->getVelocity(), negNormal);
                
                // Small damage on collision
                int damage = enemySize / 5;
                player->takeDamage(damage);
                if (damage > 0) {
                    particles->spawnDamageNumber(playerPos, damage, false);
                }
            }
        }

        ++it;
    }
    
    // Check enemy-enemy collisions (eating each other)
    for (size_t i = 0; i < enemies.size(); i++) {
        Enemy* e1 = enemies[i];
        if (!e1->isAlive()) continue;
        
        for (size_t j = i + 1; j < enemies.size(); j++) {
            Enemy* e2 = enemies[j];
            if (!e2->isAlive()) continue;
            
            Vector2 pos1 = e1->getPosition();
            Vector2 pos2 = e2->getPosition();
            int size1 = e1->getSize();
            int size2 = e2->getSize();
            
            float dx = fabs(pos1.x - pos2.x);
            float dy = fabs(pos1.y - pos2.y);
            float combinedHalfSize = (size1 + size2) / 2.0f;
            
            if (dx < combinedHalfSize && dy < combinedHalfSize) {
                // BOUNCING types don't eat, they just deal damage
                if (e1->getType() == EnemyType::BOUNCING || e2->getType() == EnemyType::BOUNCING) {
                    // Already handled in update with applyBouncingDamage
                    continue;
                }
                
                // Check for vulnerable enemies (CHASING/FLOATING with <30% health)
                bool e1Vulnerable = (e1->getType() == EnemyType::CHASING || e1->getType() == EnemyType::FLOATING) 
                                    && e1->isVulnerable();
                bool e2Vulnerable = (e2->getType() == EnemyType::CHASING || e2->getType() == EnemyType::FLOATING) 
                                    && e2->isVulnerable();
                
                // Check if one can eat the other
                if ((size1 > size2 || e2Vulnerable) && !(e1->getType() == EnemyType::STATIONARY && size1 < size2)) {
                    // e1 eats e2 (STATIONARY can eat if bigger, but not if smaller)
                    e1->growByArea(size2);
                    e2->takeDamage(e2->getHealth());
                    particles->spawnPixelExplosion(pos2, e2->getColor(), 8);
                    particles->spawnTextPopup(pos2, "EATEN", {255, 100, 100, 255});
                } else if ((size2 > size1 || e1Vulnerable) && !(e2->getType() == EnemyType::STATIONARY && size2 < size1)) {
                    // e2 eats e1
                    e2->growByArea(size1);
                    e1->takeDamage(e1->getHealth());
                    particles->spawnPixelExplosion(pos1, e1->getColor(), 8);
                    particles->spawnTextPopup(pos1, "EATEN", {255, 100, 100, 255});
                }
                // If sizes are similar and neither is vulnerable, rigid body collision handles it
            }
        }
    }
    
    // Process STATIONARY enemies eating bullets
    for (auto* enemy : enemies) {
        if (enemy->isAlive() && enemy->getType() == EnemyType::STATIONARY) {
            enemy->tryEatBullet(bullets);
        }
    }
}

// Note: Vector2Length and Vector2Normalize are defined as inline functions in game.h


void Game::updateUserMenu() {
    // User menu is handled by UI
    int selection = ui->getUserMenuSelection();
    int userSelection = ui->getUserSelection();
    int deleteConfirm = ui->getDeleteUserConfirm();

    if (selection >= 0 || userSelection >= 0 || deleteConfirm >= 0) {
        if (userSelection >= 0 && userSelection < UserManager::MAX_USERS) {
            // User selected - switch to this user
            userManager->setCurrentUser(userSelection);
            audio->playButtonClickSound();
            state = GameState::MENU;
            ui->resetTransition();
        } else if (selection == 1) {
            // Create new user - go to name input
            audio->playButtonClickSound();
            state = GameState::NAME_INPUT;
            ui->resetTransition();
            // Reset input buffer
            nameInputBuffer[0] = '\0';
        } else if (selection == 2) {
            // Back to main menu
            audio->playButtonClickSound();
            state = GameState::MENU;
            ui->resetTransition();
        } else if (deleteConfirm == 1) {
            // Confirm delete user
            int userToDelete = ui->userToDelete;  // Get stored user index
            if (userToDelete >= 0 && userToDelete < UserManager::MAX_USERS) {
                userManager->deleteUser(userToDelete);
                audio->playButtonClickSound();
                ui->resetTransition();
                // Clear delete confirmation
                ui->deleteUserConfirm = -1;
                ui->userToDelete = -1;
            }
        }

        ui->clearSelections();
    }
}

void Game::drawUserMenu() {
    // Draw user menu UI
    ui->drawUserMenu(userManager);
}

void Game::updateNameInput() {
    // Handle character input for username
    int key = GetCharPressed();

    if (key != 0) {
        // Get current length
        int len = 0;
        while (nameInputBuffer[len] != '\0' && len < 63) {
            len++;
        }

        if (key == KEY_BACKSPACE) {
            // Handle backspace
            if (len > 0) {
                nameInputBuffer[len - 1] = '\0';
            }
        } else if (key == KEY_ENTER) {
            // Confirm name input - create user
            if (len > 0) {
                userManager->createUser(nameInputBuffer);
                audio->playButtonClickSound();
                state = GameState::USER_MENU;
                ui->resetTransition();
            }
        } else if (key >= 32 && key <= 126 && len < 63) {
            // Add printable character (space to ~)
            nameInputBuffer[len] = (char)key;
            nameInputBuffer[len + 1] = '\0';
        }
    }

    // Handle back button to cancel
    int touchCount = GetTouchPointCount();
    if (touchCount > 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = touchCount > 0 ? GetTouchPosition(0) : GetMousePosition();
        // Back button area: top-right corner
        if (pos.x >= SCREEN_WIDTH - 100 && pos.x <= SCREEN_WIDTH - 50 &&
            pos.y >= 30 && pos.y <= 80) {
            state = GameState::USER_MENU;
            ui->resetTransition();
            audio->playButtonClickSound();
        }
    }
}

void Game::drawNameInput() {
    // Draw name input screen
    ui->drawNameInput(nameInputBuffer);
}

void Game::quickSave() {
    // Quick save during gameplay
    const float SAVE_COOLDOWN = 2.0f;  // Seconds between saves

    // Check if enough time passed since last save
    if (timeSinceLastSave < SAVE_COOLDOWN) {
        // Update user stats with current game progress
        User* user = userManager->getCurrentUser();
        if (user) {
            userManager->updateStats(mode, score, gameTime, player->getLevel());
        }

        // Reset save timer
        timeSinceLastSave = 0;
        hasRecentSave = true;

        // Show save notification
        particles->spawnTextPopup(playerPos(), "GAME SAVED!", {100, 255, 100, 255});
        audio->playButtonClickSound();

        TraceLog(LOG_INFO, "Game saved successfully");
    }
}
