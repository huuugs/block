#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"

namespace BlockEater {

class GameCamera {
public:
    GameCamera();
    ~GameCamera();

    void init();
    void update(Vector2 targetPosition, float dt);
    void apply();  // BeginMode2D
    void end();    // EndMode2D

    Camera2D* getCamera() { return &camera; }
    Rectangle getVisibleBounds() const;  // Get visible area in world coordinates

private:
    Camera2D camera;
    float smoothFactor;
};

} // namespace BlockEater

#endif // CAMERA_H
