#include "enemy.h"
#include "bullet.h"
#include <cstdlib>
#include <cmath>

namespace BlockEater {

Enemy::Enemy(EnemyType t, Vector2 pos, int startSize)
    : position(pos)
    , velocity{0, 0}
    , acceleration{0, 0}
    , size(startSize)
    , type(t)
    , alive(true)
    , chasingState(ChasingState::CHASING)
    , blockedTimer(0)
    , vulnerableTimer(0)
    , shootTimer(0)
    , shootPhase(0)
    , phaseTime(0)
{
    updateStatsForSize();

    // Give random initial velocity for bouncing and floating enemies
    if (type == EnemyType::BOUNCING || type == EnemyType::FLOATING) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        velocity.x = cosf(angle) * speed * 0.5f;
        velocity.y = sinf(angle) * speed * 0.5f;
    }
}

Enemy::~Enemy() {
}

void Enemy::updateStatsForSize() {
    // Mass proportional to volume (size^3)
    mass = (float)(size * size * size) / 1000.0f;
    if (mass < 1.0f) mass = 1.0f;
    
    // Set stats based on size
    maxHealth = size * 2;
    if (health == 0) health = maxHealth;
    
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

void Enemy::update(float dt, Vector2 playerPos, std::vector<Bullet*>& bullets, 
                   const std::vector<Enemy*>& allEnemies) {
    if (!alive) return;

    // Update AI state
    if (type == EnemyType::CHASING) {
        checkIfBlocked(allEnemies);
        
        // Check vulnerable state (health < 30%)
        if ((float)health / maxHealth < 0.3f) {
            chasingState = ChasingState::VULNERABLE;
        } else if (chasingState == ChasingState::VULNERABLE && blockedTimer <= 0) {
            chasingState = ChasingState::CHASING;
        }
        
        // Update blocked timer
        if (blockedTimer > 0) {
            blockedTimer -= dt;
            if (blockedTimer <= 0) {
                blockedTimer = 0;
                if (chasingState == ChasingState::BLOCKED) {
                    chasingState = ChasingState::CHASING;
                }
            }
        }
    }

    // Update based on type and state
    if (type == EnemyType::CHASING && chasingState == ChasingState::BLOCKED) {
        // Temporarily act like FLOATING when blocked
        updateFloating(dt);
    } else {
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
                tryEatBullet(bullets);
                break;
            case EnemyType::BOUNCING:
                updateBouncing(dt);
                break;
        }
    }
    
    // Apply physics
    updatePhysics(dt);
    checkBounds();
}

void Enemy::updatePhysics(float dt) {
    // Apply acceleration to velocity
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    
    // Apply friction
    velocity = velocity * FRICTION;
    
    // Update position
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    
    // Reset acceleration
    acceleration = {0, 0};
}

void Enemy::applyForce(Vector2 force) {
    // F = ma, so a = F/m
    acceleration.x += force.x / mass;
    acceleration.y += force.y / mass;
}

void Enemy::applyRigidBodyCollision(Enemy* other) {
    if (!other || !other->isAlive() || other == this) return;
    
    Vector2 pos1 = position;
    Vector2 pos2 = other->getPosition();
    Vector2 vel1 = velocity;
    Vector2 vel2 = other->getVelocity();
    float m1 = mass;
    float m2 = other->getMass();
    
    // Calculate collision normal
    Vector2 normal = {pos1.x - pos2.x, pos1.y - pos2.y};
    float dist = Vector2Length(normal);
    if (dist < 0.001f) return;
    normal = Vector2Normalize(normal);
    
    // Separate overlapping entities
    float combinedRadius = (size + other->getSize()) / 2.0f;
    if (dist < combinedRadius) {
        float overlap = combinedRadius - dist;
        float sep1 = overlap * (m2 / (m1 + m2));
        float sep2 = overlap * (m1 / (m1 + m2));
        position.x += normal.x * sep1;
        position.y += normal.y * sep1;
        other->setPosition({pos2.x - normal.x * sep2, pos2.y - normal.y * sep2});
    }
    
    // Elastic collision (1D along normal)
    float v1n = vel1.x * normal.x + vel1.y * normal.y;
    float v2n = vel2.x * normal.x + vel2.y * normal.y;
    
    // Conservation of momentum and kinetic energy
    float v1nNew = (v1n * (m1 - m2) + 2 * m2 * v2n) / (m1 + m2);
    float v2nNew = (v2n * (m2 - m1) + 2 * m1 * v1n) / (m1 + m2);
    
    // Update velocities
    velocity.x += (v1nNew - v1n) * normal.x;
    velocity.y += (v1nNew - v1n) * normal.y;
    other->setVelocity({vel2.x + (v2nNew - v2n) * normal.x, 
                        vel2.y + (v2nNew - v2n) * normal.y});
}

void Enemy::applyRigidBodyCollision(float otherMass, Vector2 otherVelocity, 
                                     Vector2 collisionNormal) {
    // Elastic collision with player or other entity
    Vector2 vel1 = velocity;
    Vector2 vel2 = otherVelocity;
    float m1 = mass;
    float m2 = otherMass;
    Vector2 n = collisionNormal;
    
    // Relative velocity along normal
    Vector2 relVel = {vel1.x - vel2.x, vel1.y - vel2.y};
    float velAlongNormal = relVel.x * n.x + relVel.y * n.y;
    
    // Do not resolve if velocities are separating
    if (velAlongNormal > 0) return;
    
    // Restitution
    float e = 0.8f;
    
    // Calculate impulse scalar
    float j = -(1 + e) * velAlongNormal;
    j /= (1 / m1 + 1 / m2);
    
    // Apply impulse
    Vector2 impulse = {j * n.x, j * n.y};
    velocity.x += impulse.x / m1;
    velocity.y += impulse.y / m1;
}

void Enemy::applyBouncingDamage(Enemy* other) {
    if (!other || type != EnemyType::BOUNCING) return;
    
    // BOUNCING enemies deal massive damage and push others away
    int damage = (int)(size * 2);  // Massive damage
    other->takeDamage(damage);
    
    // Push other away strongly
    Vector2 pushDir = Vector2Normalize(other->getPosition() - position);
    float pushForce = mass * 500.0f;  // Strong push
    other->applyForce({pushDir.x * pushForce, pushDir.y * pushForce});
}

void Enemy::checkIfBlocked(const std::vector<Enemy*>& allEnemies) {
    if (type != EnemyType::CHASING) return;
    
    // Check if path to player is blocked by other enemies
    int blockCount = 0;
    Vector2 toPlayer = {0, 0};  // Will be set in updateChasing
    
    for (auto* enemy : allEnemies) {
        if (!enemy->isAlive() || enemy == this) continue;
        
        // Simple check: if other enemy is very close, consider blocked
        float dist = Vector2Length(position - enemy->getPosition());
        if (dist < (size + enemy->getSize())) {
            blockCount++;
        }
    }
    
    // If surrounded by 3+ enemies, become blocked and switch to floating
    if (blockCount >= 2 && blockedTimer <= 0) {
        blockedTimer = 3.0f;  // Float randomly for 3 seconds
        chasingState = ChasingState::BLOCKED;
    }
}

bool Enemy::isVulnerable() const {
    if (type != EnemyType::CHASING && type != EnemyType::FLOATING) return false;
    return (float)health / maxHealth < 0.3f;
}

void Enemy::draw() {
    if (!alive) return;

    Color drawColor = color;
    
    // Visual indicator for vulnerable state
    if (isVulnerable()) {
        drawColor = {200, 100, 100, 200};  // Darker, semi-transparent
    }
    // Visual indicator for blocked state
    if (blockedTimer > 0 && type == EnemyType::CHASING) {
        drawColor = {150, 150, 255, 255};  // Blue tint when blocked
    }

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
        drawColor
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
        Color eyeColor = (chasingState == ChasingState::BLOCKED) ? BLUE : WHITE;
        DrawRectangle(
            (int)position.x - size/4 - eyeSize/2,
            (int)position.y - size/4 - eyeSize/2,
            eyeSize, eyeSize,
            eyeColor
        );
        DrawRectangle(
            (int)position.x + size/4 - eyeSize/2,
            (int)position.y - size/4 - eyeSize/2,
            eyeSize, eyeSize,
            eyeColor
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
        Color hpColor = isVulnerable() ? RED : (Color){255, 50, 50, 255};
        DrawRectangle(
            (int)position.x - barWidth/2,
            (int)position.y - size/2 - 10,
            (int)(barWidth * healthPercent), barHeight,
            hpColor
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
    float oldArea = size * size;
    float eatenArea = eatenSize * eatenSize;
    float newArea = oldArea + eatenArea;
    int newSize = (int)sqrtf(newArea);
    setSize(newSize);
}

void Enemy::setSize(int newSize) {
    if (newSize < 10) newSize = 10;
    if (newSize > 300) newSize = 300;
    
    size = newSize;
    float healthPercent = (health > 0) ? (float)health / maxHealth : 1.0f;
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

void Enemy::tryEatBullet(std::vector<Bullet*>& bullets) {
    if (type != EnemyType::STATIONARY) return;
    
    for (auto* bullet : bullets) {
        if (!bullet->isAlive()) continue;
        
        float dx = fabs(position.x - bullet->getPosition().x);
        float dy = fabs(position.y - bullet->getPosition().y);
        float combinedHalfSize = (size + bullet->getSize()) / 2.0f;
        
        if (dx < combinedHalfSize && dy < combinedHalfSize) {
            // Eat the bullet and grow
            growByArea(bullet->getSize());
            bullet->kill();
            
            // Visual effect
            // (particles would be spawned in game.cpp)
            break;  // Only eat one bullet per frame
        }
    }
}

void Enemy::tryShootBullet(std::vector<Bullet*>& bullets) {
    if (type != EnemyType::FLOATING) return;
    
    phaseTime += GetFrameTime();
    
    float interval = 2.0f;
    
    if (phaseTime < PHASE_DURATION) {
        float t = phaseTime / PHASE_DURATION;
        interval = 2.0f - t * 1.5f;
    } else if (phaseTime < PHASE_DURATION * 2) {
        float t = (phaseTime - PHASE_DURATION) / PHASE_DURATION;
        interval = 0.5f + t * 1.5f;
    } else if (phaseTime < PHASE_DURATION * 3) {
        return;
    } else {
        phaseTime = 0;
        return;
    }
    
    shootTimer += GetFrameTime();
    if (shootTimer >= interval) {
        shootTimer = 0;
        
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        Vector2 dir = {cosf(angle), sinf(angle)};
        
        int damage = size / 3;
        Bullet* bullet = new Bullet(position, dir, damage, -1);
        bullets.push_back(bullet);
    }
}

void Enemy::updateFloating(float dt) {
    // Random wandering with momentum
    float angleChange = ((float)(rand() % 20 - 10)) * DEG2RAD;
    float currentAngle = atan2f(velocity.y, velocity.x);
    float newAngle = currentAngle + angleChange;
    
    // Apply steering force
    Vector2 desiredVel = {cosf(newAngle) * speed, sinf(newAngle) * speed};
    Vector2 steering = {(desiredVel.x - velocity.x) * 2.0f, 
                        (desiredVel.y - velocity.y) * 2.0f};
    
    applyForce({steering.x * mass, steering.y * mass});
}

void Enemy::updateChasing(float dt, Vector2 playerPos) {
    // Move towards player using forces
    Vector2 dir = {playerPos.x - position.x, playerPos.y - position.y};
    float dist = Vector2Length(dir);
    
    if (dist > 0.1f) {
        dir = Vector2Normalize(dir);
        
        // Desired velocity towards player
        Vector2 desiredVel = {dir.x * speed, dir.y * speed};
        
        // Steering force
        Vector2 steering = {desiredVel.x - velocity.x, desiredVel.y - velocity.y};
        steering = Vector2Normalize(steering);
        steering = steering * FORCE_MULTIPLIER;
        
        applyForce(steering);
    }
}

void Enemy::updateStationary(float dt) {
    // Slight bobbing motion
    float bob = sinf(GetTime() * 2.0f) * 0.5f;
    position.y += bob * dt;
    
    // Dampen any velocity
    velocity = velocity * 0.9f;
}

void Enemy::updateBouncing(float dt) {
    // Constant velocity, bounces off walls
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
        // Clamp to world bounds
        if (position.x < halfSize) {
            position.x = halfSize;
            velocity.x = fabsf(velocity.x) * 0.5f;
        }
        if (position.x > WORLD_WIDTH - halfSize) {
            position.x = WORLD_WIDTH - halfSize;
            velocity.x = -fabsf(velocity.x) * 0.5f;
        }
        if (position.y < halfSize) {
            position.y = halfSize;
            velocity.y = fabsf(velocity.y) * 0.5f;
        }
        if (position.y > WORLD_HEIGHT - halfSize) {
            position.y = WORLD_HEIGHT - halfSize;
            velocity.y = -fabsf(velocity.y) * 0.5f;
        }
    }
}

} // namespace BlockEater
