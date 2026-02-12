#include "skills.h"
#include "enemy.h"
#include "player.h"
#include "game.h"
#include <cstdio>
#include <cmath>

namespace BlockEater {

SkillManager::SkillManager()
    : nextBulletId(0)
    , m_isRotating(false)
    , rotateTimer(0)
    , shieldTimeLeft(0)
    , shieldDuration(1.0f)
    , shieldPosition{0, 0}
    , shieldDirection{1, 0}
    , shieldLevel(1)
    , blinkFromPos{0, 0}
    , blinkToPos{0, 0}
    , blinkTimer(0)
{
}

SkillManager::~SkillManager() {
}

void SkillManager::init() {
    // Initialize 4 skills
    skills[0] = {SkillType::ROTATE, "Rotate", "\u65cb\u8f6c", 5.0f, 0.0f, 20, {255, 150, 0, 255}};
    skills[1] = {SkillType::BLINK, "Blink", "\u95ea\u73b0", 8.0f, 0.0f, 30, {100, 200, 255, 255}};
    skills[2] = {SkillType::SHOOT, "Shoot", "\u5c04\u51fb", 3.0f, 0.0f, 0, {255, 100, 100, 255}};
    skills[3] = {SkillType::SHIELD, "Shield", "\u62a4\u76fe", 15.0f, 0.0f, 40, {100, 255, 100, 255}};
}

void SkillManager::update(float dt) {
    // Update skill cooldowns
    for (int i = 0; i < 4; i++) {
        skills[i].update(dt);
    }

    // Update rotation timer
    if (m_isRotating) {
        rotateTimer -= dt;
        if (rotateTimer <= 0) {
            m_isRotating = false;
        }
    }

    // Update shield timer
    if (shieldTimeLeft > 0) {
        shieldTimeLeft -= dt;
        if (shieldTimeLeft <= 0) {
            shieldTimeLeft = 0;
        }
    }

    // Update blink effect timer
    if (blinkTimer > 0) {
        blinkTimer -= dt;
        if (blinkTimer < 0) blinkTimer = 0;
    }
}

void SkillManager::draw() {
    // Draw skill buttons in bottom-right corner
    float buttonSize = 60.0f;
    float startX = SCREEN_WIDTH - 280.0f;
    float startY = SCREEN_HEIGHT - 80.0f;
    float spacing = 70.0f;

    for (int i = 0; i < 4; i++) {
        float x = startX + i * spacing;
        float y = startY;

        // Draw button background
        Color bgColor = skills[i].buttonColor;
        if (!skills[i].isReady()) {
            // Dim the button when on cooldown
            bgColor = {(unsigned char)(bgColor.r / 3), (unsigned char)(bgColor.g / 3), (unsigned char)(bgColor.b / 3), 200};
        }
        DrawRectangle((int)x, (int)y, (int)buttonSize, (int)buttonSize, bgColor);
        DrawRectangleLines((int)x, (int)y, (int)buttonSize, (int)buttonSize, WHITE);

        // Draw skill icon (simple circle)
        DrawCircle((int)(x + buttonSize/2), (int)(y + buttonSize/2), 15, {255, 255, 255, 200});

        // Draw cooldown overlay
        if (!skills[i].isReady()) {
            float cooldownPercent = skills[i].currentCooldown / skills[i].cooldown;
            int cooldownHeight = (int)(buttonSize * cooldownPercent);
            DrawRectangle((int)x, (int)y, (int)buttonSize, cooldownHeight, {0, 0, 0, 150});
        }

        // Draw skill number
        char numText[8];
        sprintf(numText, "%d", i + 1);
        int textWidth = MeasureText(numText, 20);
        DrawText(numText, (int)(x + buttonSize - textWidth - 5), (int)(y + buttonSize - 25), 20, WHITE);
    }
}

bool SkillManager::canUseSkill(SkillType type) const {
    int index = (int)type;
    if (index < 0 || index >= 4) return false;
    return skills[index].isReady();
}

bool SkillManager::useSkill(SkillType type, Vector2 playerPos, Vector2 facingDir, int playerSize, int& playerHP) {
    if (!canUseSkill(type)) return false;

    int index = (int)type;
    Skill& skill = skills[index];

    switch (type) {
        case SkillType::ROTATE:
            // Activate rotation - damage reduction and reflection
            m_isRotating = true;
            rotateTimer = 2.0f;  // 2 seconds
            skill.use();
            return true;

        case SkillType::BLINK: {
            // Teleport 5x player size in facing direction
            float blinkDist = playerSize * 5.0f;
            Vector2 newPos = {
                playerPos.x + facingDir.x * blinkDist,
                playerPos.y + facingDir.y * blinkDist
            };

            // Clamp to world bounds
            if (newPos.x < playerSize) newPos.x = playerSize;
            if (newPos.x > WORLD_WIDTH - playerSize) newPos.x = WORLD_WIDTH - playerSize;
            if (newPos.y < playerSize) newPos.y = playerSize;
            if (newPos.y > WORLD_HEIGHT - playerSize) newPos.y = WORLD_HEIGHT - playerSize;

            // Set blink effect for visualization
            setBlinkEffect(playerPos, newPos, 0.3f);  // 0.3 second flash effect

            // Note: The actual position update is handled by the caller
            skill.use();
            return true;
        }

        case SkillType::SHOOT: {
            // Shoot bullet - consume HP immediately, deal 3x damage
            int hpCost = 20;  // Fixed HP cost
            playerHP -= hpCost;

            // Create bullet (handled by caller)
            skill.use();
            return true;
        }

        case SkillType::SHIELD: {
            // Create shield at 1.5x player size distance in facing direction
            float shieldDist = playerSize * 1.5f;
            shieldPosition = {
                playerPos.x + facingDir.x * shieldDist,
                playerPos.y + facingDir.y * shieldDist
            };
            shieldDirection = facingDir;
            shieldLevel = 1;  // Will be set based on player level
            shieldTimeLeft = shieldDuration;  // CRITICAL FIX: Set shield duration!
            skill.use();
            return true;
        }
    }

    return false;
}

bool SkillManager::isPointInShieldArc(Vector2 point) const {
    if (!isShieldActive()) return false;
    
    // Check if point is within shield radius
    float dist = Vector2Length(point - shieldPosition);
    if (dist > SHIELD_RADIUS * 1.5f) return false;  // Allow some margin
    
    // Calculate angle from shield center to point
    float angleToPoint = atan2f(point.y - shieldPosition.y, point.x - shieldPosition.x) * RAD2DEG;
    float baseAngle = atan2f(shieldDirection.y, shieldDirection.x) * RAD2DEG;
    
    // Normalize angle difference
    float angleDiff = angleToPoint - baseAngle;
    while (angleDiff > 180) angleDiff -= 360;
    while (angleDiff < -180) angleDiff += 360;
    
    // Check if within arc (±22.5 degrees)
    return fabs(angleDiff) <= SHIELD_ARC_ANGLE / 2.0f;
}

bool SkillManager::isOnConvexSide(Vector2 point, Vector2 velocity) const {
    // Determine which side of the shield the entity is approaching from
    // The convex side is the "outside" of the arc (facing away from shield direction)
    // The concave side is the "inside" (facing towards shield direction)
    
    Vector2 toShield = shieldPosition - point;
    float dot = toShield.x * shieldDirection.x + toShield.y * shieldDirection.y;
    
    // If dot > 0, entity is on the convex side (shield is in front of them)
    // If dot < 0, entity is on the concave side (shield is behind them)
    return dot > 0;
}

bool SkillManager::checkShieldCollision(Vector2 entityPos, Vector2& entityVel, float entityMass,
                                        bool& shouldAccelerate) {
    if (!isShieldActive()) return false;
    if (!isPointInShieldArc(entityPos)) return false;
    
    // Check distance to shield
    float dist = Vector2Length(entityPos - shieldPosition);
    if (dist > SHIELD_RADIUS) return false;
    
    // Determine which side
    bool onConvex = isOnConvexSide(entityPos, entityVel);
    
    if (onConvex) {
        // Convex side: mirror reflection
        shouldAccelerate = false;
        
        // Calculate normal at collision point (pointing outward from shield center)
        Vector2 normal = Vector2Normalize(entityPos - shieldPosition);
        
        // Reflect velocity: v' = v - 2(v·n)n
        float velDotNormal = entityVel.x * normal.x + entityVel.y * normal.y;
        entityVel.x = entityVel.x - 2 * velDotNormal * normal.x;
        entityVel.y = entityVel.y - 2 * velDotNormal * normal.y;
        
        // Apply restitution (energy loss)
        entityVel = entityVel * RESTITUTION;
        
        // Push entity out of shield
        float overlap = SHIELD_RADIUS - dist;
        entityPos = entityPos + normal * (overlap + 5.0f);
        
        return true;  // Entity was reflected
    } else {
        // Concave side: pass through with acceleration
        shouldAccelerate = true;
        
        // Accelerate the entity
        float speed = Vector2Length(entityVel);
        if (speed > 0) {
            Vector2 dir = Vector2Normalize(entityVel);
            speed *= 1.5f;  // 50% speed boost
            entityVel = dir * speed;
        }
        
        return false;  // Entity passes through
    }
}

void SkillManager::processShieldInteractions(Player* player, std::vector<Enemy*>& enemies) {
    if (!isShieldActive() || !player) return;
    
    // Check collision with player
    bool shouldAccel = false;
    Vector2 playerVel = player->getVelocity();
    Vector2 playerPos = player->getPosition();
    
    if (checkShieldCollision(playerPos, playerVel, player->getMass(), shouldAccel)) {
        player->setPosition(playerPos);
        player->setVelocity(playerVel);
    }
    
    // Check collision with enemies
    for (auto* enemy : enemies) {
        if (!enemy->isAlive()) continue;
        
        Vector2 enemyVel = enemy->getVelocity();
        Vector2 enemyPos = enemy->getPosition();
        
        if (checkShieldCollision(enemyPos, enemyVel, enemy->getMass(), shouldAccel)) {
            enemy->setPosition(enemyPos);
            enemy->setVelocity(enemyVel);
            
            // Deal damage to enemy on convex collision
            if (!shouldAccel) {
                int damage = shieldLevel * 10;
                enemy->takeDamage(damage);
            }
        }
    }
}

} // namespace BlockEater
