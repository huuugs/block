#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "game.h"
#include <vector>

namespace BlockEater {

// Forward declaration
class Bullet;

// Player level stats - only affects combat stats, not size
struct LevelStats {
    int maxHealth;
    int armor;
    Color color;
    float moveSpeed;
    float maxForce;  // Maximum steering force
};

static constexpr LevelStats LEVEL_STATS[] = {
    {100, 0,   {144, 238, 144, 255}, 200, 500},   // Level 1
    {150, 2,   {0, 255, 0, 255}, 195, 550},       // Level 2
    {220, 4,   {0, 255, 127, 255}, 190, 600},     // Level 3
    {300, 6,   {0, 250, 154, 255}, 185, 650},     // Level 4
    {400, 8,   {0, 255, 255, 255}, 180, 700},     // Level 5
    {520, 10,  {0, 191, 255, 255}, 175, 750},     // Level 6
    {660, 12,  {30, 144, 255, 255}, 170, 800},    // Level 7
    {820, 15,  {65, 105, 225, 255}, 165, 850},    // Level 8
    {1000, 18, {138, 43, 226, 255}, 160, 900},    // Level 9
    {1200, 21, {148, 0, 211, 255}, 155, 950},     // Level 10
    {1420, 24, {255, 0, 255, 255}, 150, 1000},    // Level 11
    {1660, 27, {255, 20, 147, 255}, 145, 1050},   // Level 12
    {1920, 30, {255, 69, 0, 255}, 140, 1100},     // Level 13
    {2200, 33, {255, 140, 0, 255}, 135, 1150},    // Level 14
    {2500, 36, {255, 215, 0, 255}, 130, 1200}     // Level 15
};

static constexpr int MAX_LEVEL = 15;
static constexpr int INITIAL_SIZE = 30;

class Player {
public:
    Player();
    ~Player();

    void update(float dt, std::vector<Bullet*>& bullets);
    void draw();

    // Physics-based movement - joystick applies force
    void applyJoystickInput(Vector2 inputDirection);
    void takeDamage(int damage);
    void heal(int amount);
    void addExperience(int exp);
    void levelUp();

    // Getters
    Vector2 getPosition() const { return position; }
    Vector2 getVelocity() const { return velocity; }
    int getSize() const { return size; }
    float getMass() const { return mass; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return LEVEL_STATS[level - 1].maxHealth; }
    int getArmor() const { return LEVEL_STATS[level - 1].armor; }
    int getLevel() const { return level; }
    int getExperience() const { return experience; }
    int getExperienceToNextLevel() const { return experienceToNextLevel; }
    float getEnergy() const { return kineticEnergy; }
    float getMaxEnergy() const { return maxKineticEnergy; }
    Color getColor() const { return LEVEL_STATS[level - 1].color; }
    float getMoveSpeed() const { return LEVEL_STATS[level - 1].moveSpeed; }
    Vector2 getFacingDirection() const { return facingDirection; }
    
    // Setters
    void setPosition(Vector2 pos) { position = pos; }
    void setVelocity(Vector2 vel) { velocity = vel; }
    
    // Size management
    void growByArea(int eatenSize);
    void setSize(int newSize);
    
    // Bullet skill
    bool hasBulletSkill() const { return bulletSkillEnabled; }
    void enableBulletSkill() { bulletSkillEnabled = true; }
    void tryShootBullet(std::vector<Bullet*>& bullets);

    // Physics
    void applyForce(Vector2 force);
    void applyRigidBodyCollision(float otherMass, Vector2 otherVelocity, Vector2 collisionNormal);

private:
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Vector2 facingDirection;
    int size;
    float mass;  // Mass proportional to volume
    int health;
    int level;
    int experience;
    int experienceToNextLevel;
    
    // Physics-based energy system
    float kineticEnergy;  // KE = 0.5 * m * v^2
    float maxKineticEnergy;  // Maximum storable energy
    float potentialEnergy;
    float invincibleTime;
    
    // Bullet skill
    bool bulletSkillEnabled;
    float bulletCooldown;
    static constexpr float BULLET_COOLDOWN = 0.5f;
    
    // Physics constants
    static constexpr float FRICTION = 0.99f;  // Very low friction (space-like)
    static constexpr float RESTITUTION = 0.8f;  // Bounciness
    static constexpr float DENSITY = 0.01f;  // Mass per unit volume

    void checkBounds();
    void updatePhysics(float dt);
    void updateStatsForSize();
    void updateEnergy();
};

} // namespace BlockEater

#endif // PLAYER_H
