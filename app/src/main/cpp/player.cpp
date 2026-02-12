#include "player.h"
#include "bullet.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdarg>  // For va_list if needed

namespace BlockEater {

Player::Player()
    : position{WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f}
    , velocity{0, 0}
    , acceleration{0, 0}
    , facingDirection{1, 0}
    , size(INITIAL_SIZE)
    , health(LEVEL_STATS[0].maxHealth)
    , level(1)
    , experience(0)
    , experienceToNextLevel(50)
    , kineticEnergy(0)
    , maxKineticEnergy(0)
    , potentialEnergy(100.0f)
    , invincibleTime(0)
    , bulletSkillEnabled(false)
    , bulletCooldown(0)
{
    updateStatsForSize();
    updateEnergy();
}

Player::~Player() {
}

void Player::updateStatsForSize() {
    // Mass proportional to volume: m = density * size^3
    mass = DENSITY * size * size * size;
    if (mass < 1.0f) mass = 1.0f;
    
    // Max kinetic energy based on mass and max speed
    float maxSpeed = getMoveSpeed();
    maxKineticEnergy = 0.5f * mass * maxSpeed * maxSpeed;
}

void Player::updateEnergy() {
    // Kinetic energy: KE = 0.5 * m * v^2
    float speedSq = velocity.x * velocity.x + velocity.y * velocity.y;
    kineticEnergy = 0.5f * mass * speedSq;
}

void Player::update(float dt, std::vector<Bullet*>& bullets) {
    // Update invincibility
    if (invincibleTime > 0) {
        invincibleTime -= dt;
    }

    // Apply physics
    updatePhysics(dt);
    
    // Update facing direction based on velocity
    if (Vector2Length(velocity) > 1.0f) {
        facingDirection = Vector2Normalize(velocity);
    }
    
    // Update energy
    updateEnergy();
    
    // Check bounds
    checkBounds();
    
    // Bullet skill
    if (bulletCooldown > 0) {
        bulletCooldown -= dt;
        if (bulletCooldown < 0) bulletCooldown = 0;
    }
    
    if (bulletSkillEnabled) {
        tryShootBullet(bullets);
    }
}

void Player::updatePhysics(float dt) {
    // Apply acceleration to velocity (Newton's 2nd law: F = ma)
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    
    // Apply very low friction (space-like environment)
    velocity = velocity * FRICTION;
    
    // Update position
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    
    // Reset acceleration
    acceleration = {0, 0};
    
    // Regenerate potential energy when nearly stopped
    if (Vector2Length(velocity) < 5.0f && potentialEnergy < maxKineticEnergy) {
        potentialEnergy += maxKineticEnergy * 0.1f * dt;
        if (potentialEnergy > maxKineticEnergy) {
            potentialEnergy = maxKineticEnergy;
        }
    }
}

void Player::applyJoystickInput(Vector2 inputDirection) {
    // Joystick applies force, not direct velocity change
    // FIX: Removed threshold check - small inputs should still apply force

    // DEBUG: Log input vector details using TraceLog for Android logcat visibility
    float inputLen = Vector2Length(inputDirection);
    // Use integers (x1000) to avoid float formatting issues
    TraceLog(LOG_INFO, "JOYSTICK: input=%d,%d len=%d",
            (int)(inputDirection.x * 1000), (int)(inputDirection.y * 1000),
            (int)(inputLen * 1000));

    // Don't apply force if input is essentially zero
    if (inputLen < 0.01f) {
        return;
    }

    // Normalize input
    Vector2 dir = Vector2Normalize(inputDirection);

    // Calculate desired velocity based on max speed
    float maxSpeed = getMoveSpeed();
    Vector2 desiredVelocity = {dir.x * maxSpeed, dir.y * maxSpeed};

    // Steering force = desired - current
    Vector2 steering = {desiredVelocity.x - velocity.x,
                        desiredVelocity.y - velocity.y};

    // Limit steering force based on level
    float maxForce = LEVEL_STATS[level - 1].maxForce;
    float steerMag = Vector2Length(steering);
    if (steerMag > maxForce) {
        steering = Vector2Normalize(steering) * maxForce;
    }

    // Apply force (F = ma, so a = F/m)
    applyForce(steering);

    // DEBUG: Log force application
    TraceLog(LOG_INFO, "FORCE: steer=%d,%d accel=%d,%d mass=%d",
            (int)(steering.x), (int)(steering.y),
            (int)(acceleration.x), (int)(acceleration.y),
            (int)mass);

    // Consume potential energy to apply force
    float energyCost = Vector2Length(steering) * 0.1f;
    if (potentialEnergy >= energyCost) {
        potentialEnergy -= energyCost;
    } else {
        // Not enough energy - reduce force
        float ratio = potentialEnergy / energyCost;
        steering = steering * ratio;
        potentialEnergy = 0;
    }
}

void Player::applyForce(Vector2 force) {
    acceleration.x += force.x / mass;
    acceleration.y += force.y / mass;
}

void Player::applyRigidBodyCollision(float otherMass, Vector2 otherVelocity, 
                                     Vector2 collisionNormal) {
    // Elastic collision with another entity
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
    
    // Restitution (bounciness)
    float e = RESTITUTION;
    
    // Calculate impulse scalar
    float j = -(1 + e) * velAlongNormal;
    j /= (1 / m1 + 1 / m2);
    
    // Apply impulse
    Vector2 impulse = {j * n.x, j * n.y};
    velocity.x += impulse.x / m1;
    velocity.y += impulse.y / m1;
}

void Player::draw() {
    // Blink when invincible
    if (invincibleTime > 0 && fmodf(invincibleTime, 0.1f) < 0.05f) {
        return;
    }

    Color c = getColor();

    // Draw shadow
    DrawRectangle(
        (int)position.x - size/2 + 4,
        (int)position.y - size/2 + 4,
        size, size,
        {0, 0, 0, 100}
    );

    // Draw main block
    DrawRectangle(
        (int)position.x - size/2,
        (int)position.y - size/2,
        size, size,
        c
    );

    // Draw pixel border
    DrawRectangleLines(
        (int)position.x - size/2,
        (int)position.y - size/2,
        size, size,
        {255, 255, 255, 180}
    );

    // Draw highlight
    int highlightSize = size / 3;
    DrawRectangle(
        (int)position.x - size/2 + 2,
        (int)position.y - size/2 + 2,
        highlightSize, highlightSize,
        {255, 255, 255, 100}
    );

    // Draw level indicator
    char levelText[16];
    sprintf(levelText, "L%d", level);
    int fontSize = 10;
    int textWidth = MeasureText(levelText, fontSize);
    DrawText(levelText,
             (int)position.x - textWidth/2,
             (int)position.y - fontSize/2,
             fontSize, WHITE);
             
    // Draw bullet skill indicator
    if (bulletSkillEnabled) {
        DrawCircleLines((int)position.x, (int)position.y, size/2 + 5, {255, 255, 0, 150});
    }
    
    // Draw velocity indicator (small arrow)
    if (Vector2Length(velocity) > 10.0f) {
        Vector2 dir = Vector2Normalize(velocity);
        int arrowLen = size / 2 + 10;
        Vector2 end = {position.x + dir.x * arrowLen, position.y + dir.y * arrowLen};
        DrawLine((int)position.x, (int)position.y, (int)end.x, (int)end.y, 
                 {255, 255, 255, 150});
    }
}

void Player::takeDamage(int damage) {
    if (invincibleTime > 0) return;

    int armor = getArmor();
    int actualDamage = std::max(1, damage - armor);
    health -= actualDamage;

    if (health < 0) health = 0;

    invincibleTime = 0.5f;
}

void Player::heal(int amount) {
    health += amount;
    int maxHealth = getMaxHealth();
    if (health > maxHealth) health = maxHealth;
}

void Player::addExperience(int exp) {
    experience += exp;

    while (experience >= experienceToNextLevel && level < MAX_LEVEL) {
        experience -= experienceToNextLevel;
        levelUp();
    }
}

void Player::levelUp() {
    if (level < MAX_LEVEL) {
        level++;
        // Reduced experience requirement: 20 * level * level instead of 50 * level * level
        experienceToNextLevel = 20 * level * level;

        // Restore health on level up
        int maxHp = getMaxHealth();
        health = maxHp;

        // Restore potential energy
        potentialEnergy = maxKineticEnergy;

        // Update stats for new level
        updateStatsForSize();
    }
}

void Player::growByArea(int eatenSize) {
    float oldArea = size * size;
    float eatenArea = eatenSize * eatenSize;
    float newArea = oldArea + eatenArea;
    int newSize = (int)sqrtf(newArea);
    setSize(newSize);
}

void Player::setSize(int newSize) {
    if (newSize < 10) newSize = 10;
    if (newSize > 500) newSize = 500;
    
    size = newSize;
    
    float healthPercent = (health > 0) ? (float)health / getMaxHealth() : 1.0f;
    updateStatsForSize();
    health = (int)(getMaxHealth() * healthPercent);
    if (health < 1) health = 1;
}

void Player::tryShootBullet(std::vector<Bullet*>& bullets) {
    if (!bulletSkillEnabled || bulletCooldown > 0) return;
    
    Bullet* bullet = new Bullet(position, facingDirection, size, 0);
    bullets.push_back(bullet);
    
    // Recoil
    Vector2 recoil = {-facingDirection.x * 50.0f, -facingDirection.y * 50.0f};
    applyForce(recoil);
    
    bulletCooldown = BULLET_COOLDOWN;
}

void Player::checkBounds() {
    int halfSize = size / 2;
    bool hitWall = false;
    Vector2 normal = {0, 0};

    if (position.x < halfSize) {
        position.x = halfSize;
        normal.x = 1;
        hitWall = true;
    }
    if (position.x > WORLD_WIDTH - halfSize) {
        position.x = WORLD_WIDTH - halfSize;
        normal.x = -1;
        hitWall = true;
    }
    if (position.y < halfSize) {
        position.y = halfSize;
        normal.y = 1;
        hitWall = true;
    }
    if (position.y > WORLD_HEIGHT - halfSize) {
        position.y = WORLD_HEIGHT - halfSize;
        normal.y = -1;
        hitWall = true;
    }
    
    // Reflect velocity off walls
    if (hitWall && Vector2Length(normal) > 0) {
        normal = Vector2Normalize(normal);
        float velDotNormal = velocity.x * normal.x + velocity.y * normal.y;
        if (velDotNormal < 0) {
            velocity.x -= 2 * velDotNormal * normal.x * RESTITUTION;
            velocity.y -= 2 * velDotNormal * normal.y * RESTITUTION;
        }
    }
}

} // namespace BlockEater
