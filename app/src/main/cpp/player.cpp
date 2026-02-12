#include "player.h"
#include "bullet.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace BlockEater {

Player::Player()
    : position{WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f}
    , velocity{0, 0}
    , facingDirection{1, 0}  // Default facing right
    , size(INITIAL_SIZE)  // Start with initial size
    , health(LEVEL_STATS[0].maxHealth)
    , level(1)
    , experience(0)
    , experienceToNextLevel(50)
    , energy(100.0f)
    , maxEnergy(100.0f)
    , invincibleTime(0)
    , bulletSkillEnabled(false)
    , bulletCooldown(0)
{
}

Player::~Player() {
}

void Player::update(float dt, std::vector<Bullet*>& bullets) {
    // Update invincibility frames
    if (invincibleTime > 0) {
        invincibleTime -= dt;
    }

    // Apply velocity with damping
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    // Apply friction
    velocity = velocity * 0.92f;

    // Energy regeneration when not moving
    if (Vector2Length(velocity) < 1.0f && energy < maxEnergy) {
        energy += 5.0f * dt;
        if (energy > maxEnergy) energy = maxEnergy;
    }

    // Check bounds
    checkBounds();
    
    // Update bullet skill
    updateBulletSkill(dt);
    
    // Try to shoot bullets if skill enabled
    if (bulletSkillEnabled) {
        tryShootBullet(bullets);
    }
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

    // Draw main block with pixel style
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
        // Draw small glow around player
        DrawCircleLines((int)position.x, (int)position.y, size/2 + 5, {255, 255, 0, 150});
    }
}

void Player::move(Vector2 direction) {
    float speed = getMoveSpeed();
    float len = Vector2Length(direction);

    if (len > 0.01f) {
        // Normalize and apply speed
        Vector2 dir = Vector2Normalize(direction);
        velocity.x += dir.x * speed * 0.1f;
        velocity.y += dir.y * speed * 0.1f;

        // Update facing direction when moving
        facingDirection = dir;

        // Clamp velocity
        float velLen = Vector2Length(velocity);
        float maxVel = speed;
        if (velLen > maxVel) {
            velocity = Vector2Normalize(velocity) * maxVel;
        }

        // Consume energy
        energy -= 10.0f * GetFrameTime();
        if (energy < 0) energy = 0;
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
        // Reduced experience requirement for faster leveling
        experienceToNextLevel = 50 * level * level;

        // Restore health on level up
        int maxHp = getMaxHealth();
        health = maxHp;

        // Restore energy
        energy = maxEnergy;
    }
}

void Player::growByArea(int eatenSize) {
    // Calculate new size based on area: A_new = A_old + A_eaten
    // size is diameter, so area is proportional to size^2
    float oldArea = size * size;
    float eatenArea = eatenSize * eatenSize;
    float newArea = oldArea + eatenArea;
    int newSize = (int)sqrtf(newArea);
    
    setSize(newSize);
}

void Player::setSize(int newSize) {
    if (newSize < 10) newSize = 10;
    if (newSize > 500) newSize = 500;  // Max size cap
    
    size = newSize;
    
    // Health is kept proportional to max health when size changes
    float healthPercent = (float)health / getMaxHealth();
    health = (int)(getMaxHealth() * healthPercent);
    if (health < 1) health = 1;
}

void Player::tryShootBullet(std::vector<Bullet*>& bullets) {
    if (!bulletSkillEnabled || bulletCooldown > 0) return;
    
    // Shoot in facing direction
    Bullet* bullet = new Bullet(position, facingDirection, size, 0);  // playerId = 0
    bullets.push_back(bullet);
    
    bulletCooldown = BULLET_COOLDOWN;
}

void Player::updateBulletSkill(float dt) {
    if (bulletCooldown > 0) {
        bulletCooldown -= dt;
        if (bulletCooldown < 0) bulletCooldown = 0;
    }
}

void Player::checkBounds() {
    int halfSize = size / 2;

    if (position.x < halfSize) {
        position.x = halfSize;
        velocity.x = 0;
    }
    if (position.x > WORLD_WIDTH - halfSize) {
        position.x = WORLD_WIDTH - halfSize;
        velocity.x = 0;
    }
    if (position.y < halfSize) {
        position.y = halfSize;
        velocity.y = 0;
    }
    if (position.y > WORLD_HEIGHT - halfSize) {
        position.y = WORLD_HEIGHT - halfSize;
        velocity.y = 0;
    }
}

} // namespace BlockEater
