#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

// Player level stats (expanded to 15 levels)
struct LevelStats {
    int size;
    int maxHealth;
    int armor;
    Color color;
    float moveSpeed;
};

static constexpr LevelStats LEVEL_STATS[] = {
    {30, 100, 0,   {144, 238, 144, 255}, 200},   // Level 1 - Light Green
    {35, 150, 2,   {0, 255, 0, 255}, 195},       // Level 2 - Green
    {40, 220, 4,   {0, 255, 127, 255}, 190},     // Level 3 - Spring Green
    {45, 300, 6,   {0, 250, 154, 255}, 185},     // Level 4 - Medium Spring Green
    {50, 400, 8,   {0, 255, 255, 255}, 180},     // Level 5 - Cyan
    {56, 520, 10,  {0, 191, 255, 255}, 175},     // Level 6 - Deep Sky Blue
    {62, 660, 12,  {30, 144, 255, 255}, 170},    // Level 7 - Dodger Blue
    {68, 820, 15,  {65, 105, 225, 255}, 165},    // Level 8 - Royal Blue
    {75, 1000, 18, {138, 43, 226, 255}, 160},    // Level 9 - Blue Violet
    {82, 1200, 21, {148, 0, 211, 255}, 155},     // Level 10 - Dark Violet
    {90, 1420, 24, {255, 0, 255, 255}, 150},     // Level 11 - Magenta
    {98, 1660, 27, {255, 20, 147, 255}, 145},    // Level 12 - Deep Pink
    {106, 1920, 30, {255, 69, 0, 255}, 140},     // Level 13 - Orange Red
    {115, 2200, 33, {255, 140, 0, 255}, 135},    // Level 14 - Dark Orange
    {125, 2500, 36, {255, 215, 0, 255}, 130}     // Level 15 - Gold
};

static constexpr int MAX_LEVEL = 15;

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
