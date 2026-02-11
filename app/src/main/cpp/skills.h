#ifndef SKILLS_H
#define SKILLS_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

// Skill types
enum class SkillType {
    ROTATE,     // Damage reduction and reflection
    BLINK,      // Teleport in facing direction
    SHOOT,      // Shoot bullet (consumes HP)
    SHIELD      // Arc shield defense
};

// Skill data
struct Skill {
    SkillType type;
    const char* name;
    const char* nameCN;  // Chinese name
    float cooldown;
    float currentCooldown;
    int energyCost;
    Color buttonColor;

    bool isReady() const { return currentCooldown <= 0; }
    void update(float dt) {
        if (currentCooldown > 0) currentCooldown -= dt;
    }
    void use() {
        currentCooldown = cooldown;
    }
};

class SkillManager {
public:
    SkillManager();
    ~SkillManager();

    void init();
    void update(float dt);
    void draw();

    bool canUseSkill(SkillType type) const;
    bool useSkill(SkillType type, Vector2 playerPos, Vector2 facingDir, int playerSize, int& playerHP);

    float getShieldDuration() const { return shieldDuration; }
    void setShieldDuration(float duration) { shieldDuration = duration; }
    bool isShieldActive() const { return shieldTimeLeft > 0; }
    Vector2 getShieldPosition() const { return shieldPosition; }
    Vector2 getShieldDirection() const { return shieldDirection; }
    int getShieldLevel() const { return shieldLevel; }

private:
    Skill skills[4];
    int nextBulletId;
    bool isRotating;
    float rotateTimer;
    float shieldTimeLeft;
    float shieldDuration;
    Vector2 shieldPosition;
    Vector2 shieldDirection;
    int shieldLevel;
};

} // namespace BlockEater

#endif // SKILLS_H
