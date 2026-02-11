#include "bullet.h"
#include <cmath>

namespace BlockEater {

Bullet::Bullet(Vector2 pos, Vector2 dir, int dmg, int id)
    : position(pos)
    , velocity{0, 0}
    , size(10)
    , damage(dmg)
    , playerId(id)
    , lifetime(3.0f)  // 3 seconds max lifetime
    , alive(true)
    , color{255, 255, 0, 255}  // Yellow
{
    // Normalize direction and set velocity
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.001f) {
        velocity.x = (dir.x / len) * 400.0f;  // Bullet speed
        velocity.y = (dir.y / len) * 400.0f;
    }
}

Bullet::~Bullet() {
}

void Bullet::update(float dt) {
    if (!alive) return;

    // Move bullet
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    // Decrease lifetime
    lifetime -= dt;
    if (lifetime <= 0) {
        alive = false;
        return;
    }

    // Check world bounds
    if (position.x < 0 || position.x > WORLD_WIDTH ||
        position.y < 0 || position.y > WORLD_HEIGHT) {
        alive = false;
    }
}

void Bullet::draw() {
    if (!alive) return;

    // Draw bullet with glow effect
    DrawCircle((int)position.x, (int)position.y, size, {255, 255, 100, 150});  // Outer glow
    DrawCircle((int)position.x, (int)position.y, size - 2, color);  // Core

    // Draw trail
    for (int i = 1; i <= 3; i++) {
        float trailAlpha = 100 - i * 30;
        float trailSize = size - i * 2;
        Vector2 trailPos = {
            position.x - velocity.x * 0.01f * i,
            position.y - velocity.y * 0.01f * i
        };
        DrawCircle((int)trailPos.x, (int)trailPos.y, (int)trailSize, {255, 255, 0, (unsigned char)trailAlpha});
    }
}

} // namespace BlockEater
