#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

// Enemy types
enum class EnemyType {
    FLOATING,    // Random floating
    CHASING,     // Chases player
    STATIONARY,  // Stationary food
    BOUNCING     // Bounces off walls
};

class Enemy {
public:
    Enemy(EnemyType type, Vector2 pos, int size);
    ~Enemy();

    void update(float dt, Vector2 playerPos);
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

    // Setters
    void setPosition(Vector2 pos) { position = pos; }
    void takeDamage(int dmg);
    void kill() { alive = false; }

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

    void updateFloating(float dt);
    void updateChasing(float dt, Vector2 playerPos);
    void updateStationary(float dt);
    void updateBouncing(float dt);
    void checkBounds();
};

} // namespace BlockEater

#endif // ENEMY_H
