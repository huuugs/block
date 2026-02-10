#ifndef MODES_H
#define MODES_H

#include "raylib.h"
#include "game.h"
#include "player.h"

namespace BlockEater {

// Level definition for Level Mode
struct LevelDefinition {
    int levelNumber;
    int targetScore;
    int targetLevel;
    float timeLimit;
    int enemyCount;
    float spawnRate;
    char description[64];
};

class GameModeManager {
public:
    GameModeManager();
    ~GameModeManager();

    void init(GameMode mode);
    void update(float dt);
    void reset();

    // Getters
    bool isComplete() const { return complete; }
    bool isFailed() const { return failed; }
    float getTimeRemaining() const { return timeRemaining; }
    int getCurrentLevel() const { return currentLevelIndex; }
    LevelDefinition getCurrentLevelDef() const;

    // Level mode specific
    void nextLevel();
    bool hasNextLevel() const;

private:
    GameMode currentMode;
    bool complete;
    bool failed;
    float timeRemaining;
    int currentLevelIndex;
    float spawnTimer;
    float difficultyMultiplier;

    void updateEndless(float dt);
    void updateLevelMode(float dt);
    void updateTimeChallenge(float dt);

    void spawnEnemy();
    int getEnemyCountForLevel(int level);
    float getSpawnRateForLevel(int level);

    static constexpr LevelDefinition LEVELS[] = {
        {1, 500, 2, 0, 5, 2.0f, "Reach level 2"},
        {2, 1000, 3, 0, 8, 1.8f, "Reach level 3"},
        {3, 2000, 3, 120, 10, 1.5f, "Score 2000 in 2 minutes"},
        {4, 3000, 4, 0, 12, 1.3f, "Reach level 4"},
        {5, 5000, 4, 180, 15, 1.2f, "Score 5000 in 3 minutes"},
        {6, 7000, 5, 0, 18, 1.0f, "Reach level 5"},
        {7, 10000, 5, 240, 20, 0.9f, "Score 10000 in 4 minutes"},
        {8, 15000, 6, 0, 25, 0.8f, "Reach max level"},
        {9, 20000, 6, 300, 30, 0.7f, "Score 20000 in 5 minutes"},
        {10, 30000, 6, 180, 40, 0.5f, "Ultimate challenge!"}
    };

    static constexpr int LEVEL_COUNT = 10;
};

} // namespace BlockEater

#endif // MODES_H
