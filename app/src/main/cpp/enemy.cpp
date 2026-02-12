#include "enemy.h"
#include "bullet.h"
#include <cstdlib>
#include <cmath>

namespace BlockEater {

Enemy::Enemy(EnemyType type, Vector2 pos, int startSize)
    : position(pos)
    , velocity{0, 0}
    , size(startSize)
    , type(type)
    , alive(true)
    , shootTimer(0)
    , shootPhase(0)
    , phaseTime(0)
    , separationForce{0, 0}
{
    updateStatsForSize();

    // Give random initial velocity for bouncing and floating enemies
    if (type == EnemyType::BOUNCING || type == EnemyType::FLOATING) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        velocity.x = cosf(angle) * speed;
        velocity.y = sinf(angle) * speed;
    }
}

Enemy::~Enemy() {
}

void Enemy::updateStatsForSize() {
    // Set stats based on size
    maxHealth = size * 2;
    if (health == 0) health = maxHealth;  // Initial setup
    
    // Set color and speed based on type
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
}

void Enemy::update(float dt, Vector2 playerPos, std::vector<Bullet*>& bullets) {
    if (!alive) return;

    // Store old velocity magnitude for floating enemies
    float oldSpeed = Vector2Length(velocity);

    switch (type) {
        case EnemyType::FLOATING:
            updateFloating(dt);
            tryShootBullet(bullets);
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
    
    // Apply separation force (avoid overlapping)
    position.x += separationForce.x * dt;
    position.y += separationForce.y * dt;
    separationForce = {0, 0};  // Reset for next frame

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
    
    // Draw health bar above enemy
    if (health < maxHealth) {
        int barWidth = size;
        int barHeight = 4;
        float healthPercent = (float)health / maxHealth;
        DrawRectangle(
            (int)position.x - barWidth/2,
            (int)position.y - size/2 - 10,
            barWidth, barHeight,
            {50, 50, 50, 200}
        );
        DrawRectangle(
            (int)position.x - barWidth/2,
            (int)position.y - size/2 - 10,
            (int)(barWidth * healthPercent), barHeight,
            {255, 50, 50, 255}
        );
    }
}

void Enemy::takeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        alive = false;
    }
}

void Enemy::growByArea(int eatenSize) {
    // Calculate new size based on area: A_new = A_old + A_eaten
    // size is diameter, so area is proportional to size^2
    float oldArea = size * size;
    float eatenArea = eatenSize * eatenSize;
    float newArea = oldArea + eatenArea;
    int newSize = (int)sqrtf(newArea);
    
    setSize(newSize);
}

void Enemy::setSize(int newSize) {
    if (newSize < 10) newSize = 10;
    if (newSize > 300) newSize = 300;  // Max size cap
    
    size = newSize;
    
    // Update health proportionally but keep percentage
    float healthPercent = (float)health / maxHealth;
    updateStatsForSize();
    health = (int)(maxHealth * healthPercent);
    if (health < 1) health = 1;
}

bool Enemy::checkCollisionWith(const Enemy* other) const {
    if (!other || !other->isAlive() || other == this) return false;
    
    float dx = fabs(position.x - other->getPosition().x);
    float dy = fabs(position.y - other->getPosition().y);
    float combinedHalfSize = (size + other->getSize()) / 2.0f;
    
    return (dx < combinedHalfSize && dy < combinedHalfSize);
}

void Enemy::tryShootBullet(std::vector<Bullet*>& bullets) {
    if (type != EnemyType::FLOATING) return;
    
    // Update phase timer
    phaseTime += GetFrameTime();
    
    // Three phases: decrease interval (0-5s), increase interval (5-10s), pause (10-15s)
    float interval = 2.0f;  // Base interval
    
    if (phaseTime < PHASE_DURATION) {
        // Phase 1: decreasing interval (fast shooting)
        float t = phaseTime / PHASE_DURATION;
        interval = 2.0f - t * 1.5f;  // 2.0 -> 0.5 seconds
    } else if (phaseTime < PHASE_DURATION * 2) {
        // Phase 2: increasing interval (slow shooting)
        float t = (phaseTime - PHASE_DURATION) / PHASE_DURATION;
        interval = 0.5f + t * 1.5f;  // 0.5 -> 2.0 seconds
    } else if (phaseTime < PHASE_DURATION * 3) {
        // Phase 3: pause (no shooting)
        return;
    } else {
        // Reset phase
        phaseTime = 0;
        return;
    }
    
    // Try to shoot
    shootTimer += GetFrameTime();
    if (shootTimer >= interval) {
        shootTimer = 0;
        
        // Random direction
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        Vector2 dir = {cosf(angle), sinf(angle)};
        
        // Create bullet (playerId = -1 for enemy bullets)
        int damage = size / 3;
        Bullet* bullet = new Bullet(position, dir, damage, -1);
        bullets.push_back(bullet);
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
        if (position.x < halfSize || position.x > WORLD_WIDTH - halfSize) {
            velocity.x = -velocity.x;
            position.x = fmaxf(halfSize, fminf(WORLD_WIDTH - halfSize, position.x));
        }
        if (position.y < halfSize || position.y > WORLD_HEIGHT - halfSize) {
            velocity.y = -velocity.y;
            position.y = fmaxf(halfSize, fminf(WORLD_HEIGHT - halfSize, position.y));
        }
    } else {
        // All enemies clamp to world bounds (no wrap around)
        position.x = fmaxf(halfSize, fminf(WORLD_WIDTH - halfSize, position.x));
        position.y = fmaxf(halfSize, fminf(WORLD_HEIGHT - halfSize, position.y));
        
        // Stop velocity if hitting boundary (for non-bouncing types)
        if (position.x <= halfSize || position.x >= WORLD_WIDTH - halfSize) {
            velocity.x = 0;
        }
        if (position.y <= halfSize || position.y >= WORLD_HEIGHT - halfSize) {
            velocity.y = 0;
        }
    }
}

} // namespace BlockEater
