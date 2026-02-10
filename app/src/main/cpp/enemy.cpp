#include "enemy.h"
#include <cstdlib>
#include <cmath>

namespace BlockEater {

Enemy::Enemy(EnemyType type, Vector2 pos, int size)
    : position(pos)
    , velocity{0, 0}
    , size(size)
    , type(type)
    , alive(true)
{
    // Set stats based on size
    maxHealth = size * 2;
    health = maxHealth;

    // Set color based on type
    switch (type) {
        case EnemyType::FLOATING:
            color = {255, 100, 100, 255};  // Red
            speed = 50.0f;
            expValue = size / 2;
            break;
        case EnemyType::CHASING:
            color = {255, 50, 50, 255};    // Dark Red
            speed = 80.0f;
            expValue = size;
            break;
        case EnemyType::STATIONARY:
            color = {100, 255, 100, 255};  // Green
            speed = 0.0f;
            expValue = size / 3;
            break;
        case EnemyType::BOUNCING:
            color = {255, 150, 50, 255};   // Orange
            speed = 120.0f;
            expValue = size / 2 + 10;
            break;
    }

    // Give random initial velocity for bouncing enemies
    if (type == EnemyType::BOUNCING) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        velocity.x = cosf(angle) * speed;
        velocity.y = sinf(angle) * speed;
    } else if (type == EnemyType::FLOATING) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        velocity.x = cosf(angle) * speed;
        velocity.y = sinf(angle) * speed;
    }
}

Enemy::~Enemy() {
}

void Enemy::update(float dt, Vector2 playerPos) {
    if (!alive) return;

    switch (type) {
        case EnemyType::FLOATING:
            updateFloating(dt);
            break;
        case EnemyType::CHASING:
            updateChasing(dt, playerPos);
            break;
        case EnemyType::STATIONARY:
            updateStationary(dt);
            break;
        case EnemyType::BOUNCING:
            updateBouncing(dt);
            break;
    }

    checkBounds();
}

void Enemy::draw() {
    if (!alive) return;

    // Draw shadow
    DrawRectangle(
        (int)position.x - size/2 + 3,
        (int)position.y - size/2 + 3,
        size, size,
        {0, 0, 0, 80}
    );

    // Draw enemy block
    DrawRectangle(
        (int)position.x - size/2,
        (int)position.y - size/2,
        size, size,
        color
    );

    // Draw pixel border
    DrawRectangleLines(
        (int)position.x - size/2,
        (int)position.y - size/2,
        size, size,
        {255, 255, 255, 150}
    );

    // Draw eyes for chasing enemies
    if (type == EnemyType::CHASING) {
        int eyeSize = size / 5;
        DrawRectangle(
            (int)position.x - size/4 - eyeSize/2,
            (int)position.y - size/4 - eyeSize/2,
            eyeSize, eyeSize,
            WHITE
        );
        DrawRectangle(
            (int)position.x + size/4 - eyeSize/2,
            (int)position.y - size/4 - eyeSize/2,
            eyeSize, eyeSize,
            WHITE
        );
    }
}

void Enemy::takeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        alive = false;
    }
}

void Enemy::updateFloating(float dt) {
    // Slowly change direction
    float angleChange = ((float)(rand() % 20 - 10)) * DEG2RAD * dt;
    float currentAngle = atan2f(velocity.y, velocity.x);
    float newAngle = currentAngle + angleChange;

    velocity.x = cosf(newAngle) * speed;
    velocity.y = sinf(newAngle) * speed;

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}

void Enemy::updateChasing(float dt, Vector2 playerPos) {
    // Move towards player
    Vector2 dir = {
        playerPos.x - position.x,
        playerPos.y - position.y
    };

    float dist = Vector2Length(dir);
    if (dist > 0.1f) {
        dir = Vector2Normalize(dir);
        velocity.x = dir.x * speed;
        velocity.y = dir.y * speed;
    }

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}

void Enemy::updateStationary(float dt) {
    // Slight bobbing motion
    float bob = sinf(GetTime() * 2.0f) * 0.5f;
    position.y += bob * dt;
}

void Enemy::updateBouncing(float dt) {
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}

void Enemy::checkBounds() {
    int halfSize = size / 2;

    if (type == EnemyType::BOUNCING) {
        if (position.x < halfSize || position.x > SCREEN_WIDTH - halfSize) {
            velocity.x = -velocity.x;
            position.x = fmaxf(halfSize, fminf(SCREEN_WIDTH - halfSize, position.x));
        }
        if (position.y < halfSize || position.y > SCREEN_HEIGHT - halfSize) {
            velocity.y = -velocity.y;
            position.y = fmaxf(halfSize, fminf(SCREEN_HEIGHT - halfSize, position.y));
        }
    } else {
        // Wrap around for floating enemies
        if (position.x < -size) position.x = SCREEN_WIDTH + size;
        if (position.x > SCREEN_WIDTH + size) position.x = -size;
        if (position.y < -size) position.y = SCREEN_HEIGHT + size;
        if (position.y > SCREEN_HEIGHT + size) position.y = -size;
    }

    // Clamp position
    position.x = fmaxf(halfSize, fminf(SCREEN_WIDTH - halfSize, position.x));
    position.y = fmaxf(halfSize, fminf(SCREEN_HEIGHT - halfSize, position.y));
}

} // namespace BlockEater
