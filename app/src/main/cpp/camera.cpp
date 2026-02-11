#include "camera.h"

namespace BlockEater {

GameCamera::GameCamera() : smoothFactor(5.0f) {
}

GameCamera::~GameCamera() {
}

void GameCamera::init() {
    camera.target = {0, 0};
    camera.offset = {0, 0};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    smoothFactor = 5.0f;
}

void GameCamera::update(Vector2 targetPosition, float dt) {
    // Smooth follow using lerp
    Vector2 currentTarget = camera.target;
    Vector2 target = targetPosition;

    camera.target.x += (target.x - currentTarget.x) * smoothFactor * dt;
    camera.target.y += (target.y - currentTarget.y) * smoothFactor * dt;

    // Keep camera centered on screen
    camera.offset.x = 640.0f;  // SCREEN_WIDTH / 2
    camera.offset.y = 360.0f;  // SCREEN_HEIGHT / 2
}

void GameCamera::apply() {
    BeginMode2D(camera);
}

void GameCamera::end() {
    EndMode2D();
}

Rectangle GameCamera::getVisibleBounds() const {
    Rectangle bounds;
    bounds.x = camera.target.x - 640.0f;  // SCREEN_WIDTH / 2
    bounds.y = camera.target.y - 360.0f;  // SCREEN_HEIGHT / 2
    bounds.width = 1280.0f;               // SCREEN_WIDTH
    bounds.height = 720.0f;              // SCREEN_HEIGHT

    // Clamp to world bounds
    if (bounds.x < 0) bounds.x = 0;
    if (bounds.y < 0) bounds.y = 0;
    if (bounds.x + bounds.width > 5120.0f) bounds.x = 5120.0f - bounds.width;   // WORLD_WIDTH
    if (bounds.y + bounds.height > 2880.0f) bounds.y = 2880.0f - bounds.height; // WORLD_HEIGHT

    return bounds;
}

} // namespace BlockEater
