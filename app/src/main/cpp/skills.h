#ifndef SKILLS_H
#define SKILLS_H

#include "raylib.h"
#include "game.h"
#include <vector>

namespace BlockEater {

// Forward declarations from game.h (Player and Enemy are already declared there)

// Skill types
enum class SkillType {
    ROTATE,     // Damage reduction and reflection
    BLINK,      // Teleport in facing direction
    SHOOT,      // Shoot bullet (consumes HP)
    SHIELD      // Advanced shield: convex reflects, concave accelerates
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
    
    // New shield physics
    // Returns: true if entity should be reflected (convex side), 
    //          false if it should pass through (concave side) with acceleration
    bool checkShieldCollision(Vector2 entityPos, Vector2& entityVel, float entityMass, 
                              bool& shouldAccelerate);
    void processShieldInteractions(Player* player, std::vector<Enemy*>& enemies);

    // Skill visual effects
    bool isRotating() const { return m_isRotating; }
    float getRotateTimer() const { return rotateTimer; }
    Vector2 getBlinkFromPos() const { return blinkFromPos; }
    Vector2 getBlinkToPos() const { return blinkToPos; }
    float getBlinkTimer() const { return blinkTimer; }
    void setBlinkEffect(Vector2 from, Vector2 to, float duration) {
        blinkFromPos = from;
        blinkToPos = to;
        blinkTimer = duration;
    }

private:
    Skill skills[4];
    int nextBulletId;
    bool m_isRotating;
    float rotateTimer;
    float shieldTimeLeft;
    float shieldDuration;
    Vector2 shieldPosition;
    Vector2 shieldDirection;
    int shieldLevel;
    
    // Shield geometry
    static constexpr float SHIELD_RADIUS = 80.0f;
    static constexpr float SHIELD_ARC_ANGLE = 45.0f;  // Total arc width
    static constexpr float RESTITUTION = 0.9f;  // Bounciness

    // Blink effect
    Vector2 blinkFromPos;
    Vector2 blinkToPos;
    float blinkTimer;
    
    // Helper functions
    bool isPointInShieldArc(Vector2 point) const;
    bool isOnConvexSide(Vector2 point, Vector2 velocity) const;
};

} // namespace BlockEater

#endif // SKILLS_H
