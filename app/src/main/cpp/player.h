#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

// Player level stats
struct LevelStats {
    int size;
    int maxHealth;
    int armor;
    Color color;
    float moveSpeed;
};

static constexpr LevelStats LEVEL_STATS[] = {
    {30, 100, 0,   {0, 255, 0, 255}, 200},   // Level 1 - Green
    {40, 200, 5,   {0, 255, 255, 255}, 190}, // Level 2 - Cyan
    {50, 350, 10,  {0, 100, 255, 255}, 180}, // Level 3 - Blue
    {65, 550, 15,  {150, 0, 255, 255}, 170}, // Level 4 - Purple
    {80, 800, 20,  {255, 0, 150, 255}, 160}, // Level 5 - Pink
    {100, 1100, 30, {255, 215, 0, 255}, 150} // Level 6 - Gold
};

static constexpr int MAX_LEVEL = 6;

class Player {
public:
    Player();
    ~Player();

    void update(float dt);
    void draw();

    void move(Vector2 direction);
    void takeDamage(int damage);
    void heal(int amount);
    void addExperience(int exp);
    void levelUp();

    // Getters
    Vector2 getPosition() const { return position; }
    int getSize() const { return LEVEL_STATS[level - 1].size; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return LEVEL_STATS[level - 1].maxHealth; }
    int getArmor() const { return LEVEL_STATS[level - 1].armor; }
    int getLevel() const { return level; }
    float getEnergy() const { return energy; }
    float getMaxEnergy() const { return maxEnergy; }
    Color getColor() const { return LEVEL_STATS[level - 1].color; }
    float getMoveSpeed() const { return LEVEL_STATS[level - 1].moveSpeed; }

    // Setters
    void setPosition(Vector2 pos) { position = pos; }

    // Facing direction (for skills)
    Vector2 getFacingDirection() const { return facingDirection; }

private:
    Vector2 position;
    Vector2 velocity;
    Vector2 facingDirection;  // Last moved direction (for skills)
    int health;
    int level;
    int experience;
    int experienceToNextLevel;
    float energy;
    float maxEnergy;
    float invincibleTime;

    void checkBounds();
};

} // namespace BlockEater

#endif // PLAYER_H
