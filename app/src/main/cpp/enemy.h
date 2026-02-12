#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "game.h"
#include <vector>

namespace BlockEater {

// Forward declaration
class Bullet;

// Enemy types
enum class EnemyType {
    FLOATING,    // Random floating, can shoot bullets
    CHASING,     // Chases player, becomes floating when blocked
    STATIONARY,  // Stationary food, can eat bullets to grow
    BOUNCING     // Bounces off walls, deals high damage, doesn't eat
};

// AI states for CHASING type
enum class ChasingState {
    CHASING,     // Normal chasing
    BLOCKED,     // Blocked by other enemies, temporarily floating
    VULNERABLE   // Health < 30%, can be eaten by smaller enemies
};

class Enemy {
public:
    Enemy(EnemyType type, Vector2 pos, int size);
    ~Enemy();

    void update(float dt, Vector2 playerPos, std::vector<Bullet*>& bullets, 
                const std::vector<Enemy*>& allEnemies);
    void draw();

    // Getters
    Vector2 getPosition() const { return position; }
    Vector2 getVelocity() const { return velocity; }
    int getSize() const { return size; }
    float getMass() const { return mass; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    EnemyType getType() const { return type; }
    Color getColor() const { return color; }
    bool isAlive() const { return alive; }
    int getExpValue() const { return expValue; }
    float getSpeed() const { return speed; }
    bool isVulnerable() const;  // Health < 30% for CHASING/FLOATING
    bool isBlocked() const { return blockedTimer > 0; }

    // Setters
    void setPosition(Vector2 pos) { position = pos; }
    void setVelocity(Vector2 vel) { velocity = vel; }
    void takeDamage(int dmg);
    void kill() { alive = false; }
    
    // Size management - grows based on area
    void growByArea(int eatenSize);
    void setSize(int newSize);
    
    // Physics
    void applyForce(Vector2 force);
    void applyRigidBodyCollision(Enemy* other);  // Enemy-enemy collision
    void applyRigidBodyCollision(float otherMass, Vector2 otherVelocity, Vector2 collisionNormal);  // Player-enemy collision
    void applyBouncingDamage(Enemy* other);  // BOUNCING type special damage
    
    // Check collision with another enemy
    bool checkCollisionWith(const Enemy* other) const;
    
    // Bullet interaction
    void tryEatBullet(std::vector<Bullet*>& bullets);  // For STATIONARY
    void tryShootBullet(std::vector<Bullet*>& bullets);  // For FLOATING

    // CHASING AI
    void checkIfBlocked(const std::vector<Enemy*>& allEnemies);

private:
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    int size;
    float mass;  // Mass proportional to volume (size^3)
    int health;
    int maxHealth;
    EnemyType type;
    Color color;
    bool alive;
    int expValue;
    float speed;
    
    // AI state
    ChasingState chasingState;
    float blockedTimer;  // Time spent in blocked state
    float vulnerableTimer;
    
    // Bullet shooting for FLOATING enemies
    float shootTimer;
    float shootPhase;
    float phaseTime;
    static constexpr float PHASE_DURATION = 5.0f;
    
    // Physics constants
    static constexpr float FRICTION = 0.98f;
    static constexpr float FORCE_MULTIPLIER = 100.0f;

    void updateFloating(float dt);
    void updateChasing(float dt, Vector2 playerPos);
    void updateStationary(float dt);
    void updateBouncing(float dt);
    void checkBounds();
    void updatePhysics(float dt);
    void updateStatsForSize();
};

} // namespace BlockEater

#endif // ENEMY_H
