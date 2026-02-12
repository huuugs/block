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
    CHASING,     // Chases player
    STATIONARY,  // Stationary food
    BOUNCING     // Bounces off walls
};

class Enemy {
public:
    Enemy(EnemyType type, Vector2 pos, int size);
    ~Enemy();

    void update(float dt, Vector2 playerPos, std::vector<Bullet*>& bullets);
    void draw();

    // Getters
    Vector2 getPosition() const { return position; }
    int getSize() const { return size; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    EnemyType getType() const { return type; }
    Color getColor() const { return color; }
    bool isAlive() const { return alive; }
    int getExpValue() const { return expValue; }
    float getSpeed() const { return speed; }

    // Setters
    void setPosition(Vector2 pos) { position = pos; }
    void takeDamage(int dmg);
    void kill() { alive = false; }
    
    // Size management - grows based on area
    void growByArea(int eatenSize);
    void setSize(int newSize);
    
    // Check collision with another enemy (for separation)
    bool checkCollisionWith(const Enemy* other) const;
    
    // Bullet shooting for FLOATING type
    void tryShootBullet(std::vector<Bullet*>& bullets);

private:
    Vector2 position;
    Vector2 velocity;
    int size;
    int health;
    int maxHealth;
    EnemyType type;
    Color color;
    bool alive;
    int expValue;
    float speed;
    
    // Bullet shooting for FLOATING enemies
    float shootTimer;
    float shootPhase;  // For cyclic timing: decrease -> increase -> pause
    float phaseTime;
    static constexpr float PHASE_DURATION = 5.0f;  // 5 seconds per phase
    
    // Separation force
    Vector2 separationForce;

    void updateFloating(float dt);
    void updateChasing(float dt, Vector2 playerPos);
    void updateStationary(float dt);
    void updateBouncing(float dt);
    void checkBounds();
    
    // Update stats when size changes
    void updateStatsForSize();
};

} // namespace BlockEater

#endif // ENEMY_H
