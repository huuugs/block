#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "game.h"
#include <vector>

namespace BlockEater {

// Forward declaration
class Bullet;

// Player level stats (expanded to 15 levels) - only affects combat stats, not size
struct LevelStats {
    int maxHealth;
    int armor;
    Color color;
    float moveSpeed;
};

static constexpr LevelStats LEVEL_STATS[] = {
    {100, 0,   {144, 238, 144, 255}, 200},   // Level 1 - Light Green
    {150, 2,   {0, 255, 0, 255}, 195},       // Level 2 - Green
    {220, 4,   {0, 255, 127, 255}, 190},     // Level 3 - Spring Green
    {300, 6,   {0, 250, 154, 255}, 185},     // Level 4 - Medium Spring Green
    {400, 8,   {0, 255, 255, 255}, 180},     // Level 5 - Cyan
    {520, 10,  {0, 191, 255, 255}, 175},     // Level 6 - Deep Sky Blue
    {660, 12,  {30, 144, 255, 255}, 170},    // Level 7 - Dodger Blue
    {820, 15,  {65, 105, 225, 255}, 165},    // Level 8 - Royal Blue
    {1000, 18, {138, 43, 226, 255}, 160},    // Level 9 - Blue Violet
    {1200, 21, {148, 0, 211, 255}, 155},     // Level 10 - Dark Violet
    {1420, 24, {255, 0, 255, 255}, 150},     // Level 11 - Magenta
    {1660, 27, {255, 20, 147, 255}, 145},    // Level 12 - Deep Pink
    {1920, 30, {255, 69, 0, 255}, 140},      // Level 13 - Orange Red
    {2200, 33, {255, 140, 0, 255}, 135},     // Level 14 - Dark Orange
    {2500, 36, {255, 215, 0, 255}, 130}      // Level 15 - Gold
};

static constexpr int MAX_LEVEL = 15;
static constexpr int INITIAL_SIZE = 30;

class Player {
public:
    Player();
    ~Player();

    void update(float dt, std::vector<Bullet*>& bullets);
    void draw();

    void move(Vector2 direction);
    void takeDamage(int damage);
    void heal(int amount);
    void addExperience(int exp);
    void levelUp();

    // Getters
    Vector2 getPosition() const { return position; }
    int getSize() const { return size; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return LEVEL_STATS[level - 1].maxHealth; }
    int getArmor() const { return LEVEL_STATS[level - 1].armor; }
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getExperienceToNextLevel() const { return experienceToNextLevel; }
    float getEnergy() const { return energy; }
    float getMaxEnergy() const { return maxEnergy; }
    Color getColor() const { return LEVEL_STATS[level - 1].color; }
    float getMoveSpeed() const { return LEVEL_STATS[level - 1].moveSpeed; }

    // Setters
    void setPosition(Vector2 pos) { position = pos; }

    // Facing direction (for skills)
    Vector2 getFacingDirection() const { return facingDirection; }
    
    // Size management - grows based on area
    void growByArea(int eatenSize);
    void setSize(int newSize);
    
    // Bullet shooting skill (gained from eating FLOATING enemies)
    bool hasBulletSkill() const { return bulletSkillEnabled; }
    void enableBulletSkill() { bulletSkillEnabled = true; }
    void tryShootBullet(std::vector<Bullet*>& bullets);
    void updateBulletSkill(float dt);

private:
    Vector2 position;
    Vector2 velocity;
    Vector2 facingDirection;
    int size;  // Dynamic size based on eating, not level
    int health;
    int level;
    int experience;
    int experienceToNextLevel;
    float energy;
    float maxEnergy;
    float invincibleTime;
    
    // Bullet skill
    bool bulletSkillEnabled;
    float bulletCooldown;
    static constexpr float BULLET_COOLDOWN = 0.5f;  // Shoot every 0.5 seconds when enabled

    void checkBounds();
};

} // namespace BlockEater

#endif // PLAYER_H
