#include "player.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace BlockEater {

Player::Player()
    : position{WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f}
    , velocity{0, 0}
    , facingDirection{1, 0}  // Default facing right
    , health(LEVEL_STATS[0].maxHealth)
    , level(1)
    , experience(0)
    , experienceToNextLevel(50)  // Reduced from 100 for faster initial leveling
    , energy(100.0f)
    , maxEnergy(100.0f)
    , invincibleTime(0)
{
}

Player::~Player() {
}

void Player::update(float dt) {
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
}

void Player::draw() {
    // Blink when invincible
    if (invincibleTime > 0 && fmodf(invincibleTime, 0.1f) < 0.05f) {
        return;
    }

    int size = getSize();
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
        // Reduced experience requirement for faster leveling (was 100 * level * level)
        experienceToNextLevel = 50 * level * level;

        // Restore health on level up
        int maxHp = getMaxHealth();
        health = maxHp;

        // Restore energy
        energy = maxEnergy;
    }
}

void Player::checkBounds() {
    int halfSize = getSize() / 2;

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
