#include "modes.h"
#include <cstdlib>
#include <cmath>

namespace BlockEater {

GameModeManager::GameModeManager()
    : currentMode(GameMode::ENDLESS)
    , complete(false)
    , failed(false)
    , timeRemaining(0)
    , currentLevelIndex(0)
    , spawnTimer(0)
    , difficultyMultiplier(1.0f)
{
}

GameModeManager::~GameModeManager() {
}

void GameModeManager::init(GameMode mode) {
    currentMode = mode;
    reset();
}

void GameModeManager::reset() {
    complete = false;
    failed = false;
    currentLevelIndex = 0;
    spawnTimer = 0;
    difficultyMultiplier = 1.0f;

    if (currentMode == GameMode::TIME_CHALLENGE) {
        timeRemaining = 180.0f;  // 3 minutes
    } else if (currentMode == GameMode::LEVEL) {
        LevelDefinition level = LEVELS[0];
        timeRemaining = level.timeLimit;
    }
}

void GameModeManager::update(float dt) {
    if (complete || failed) return;

    switch (currentMode) {
        case GameMode::ENDLESS:
            updateEndless(dt);
            break;
        case GameMode::LEVEL:
            updateLevelMode(dt);
            break;
        case GameMode::TIME_CHALLENGE:
            updateTimeChallenge(dt);
            break;
    }
}

void GameModeManager::updateEndless(float dt) {
    // Increase difficulty over time
    difficultyMultiplier += dt * 0.01f;
    difficultyMultiplier = fminf(difficultyMultiplier, 5.0f);

    spawnTimer -= dt;
    if (spawnTimer <= 0) {
        spawnEnemy();
        spawnTimer = (2.0f / difficultyMultiplier) + 0.5f;
    }

    // Check win condition (reach max level)
    // No fail condition in endless mode
}

void GameModeManager::updateLevelMode(float dt) {
    LevelDefinition level = LEVELS[currentLevelIndex];

    // Update timer if level has time limit
    if (level.timeLimit > 0) {
        timeRemaining -= dt;
        if (timeRemaining <= 0) {
            failed = true;
            return;
        }
    }

    spawnTimer -= dt;
    if (spawnTimer <= 0) {
        spawnEnemy();
        spawnTimer = level.spawnRate;
    }

    // Check completion conditions
    // This will be checked by the game class based on score/level
}

void GameModeManager::updateTimeChallenge(float dt) {
    timeRemaining -= dt;

    if (timeRemaining <= 0) {
        complete = true;
        return;
    }

    spawnTimer -= dt;
    if (spawnTimer <= 0) {
        spawnEnemy();
        spawnTimer = 1.5f;
    }
}

void GameModeManager::spawnEnemy() {
    // This is a placeholder - actual spawning is handled by Game class
    // This function just provides timing and difficulty info
}

int GameModeManager::getEnemyCountForLevel(int level) {
    return 5 + level * 3;
}

float GameModeManager::getSpawnRateForLevel(int level) {
    return fmaxf(0.3f, 2.0f - level * 0.15f);
}

void GameModeManager::nextLevel() {
    if (hasNextLevel()) {
        currentLevelIndex++;
        LevelDefinition level = LEVELS[currentLevelIndex];
        timeRemaining = level.timeLimit;
        complete = false;
    }
}

bool GameModeManager::hasNextLevel() const {
    return currentLevelIndex < LEVEL_COUNT - 1;
}

LevelDefinition GameModeManager::getCurrentLevelDef() const {
    if (currentLevelIndex < LEVEL_COUNT) {
        return LEVELS[currentLevelIndex];
    }
    return LEVELS[0];
}

} // namespace BlockEater
